#include "Camera.h"
#include "GameObject.h"
#include "GTTime.h"
#include "Debug.h"
#include "Screen.h"
#include "Renderer.h"
#include "RenderTexture.h"
#include "ImageEffect.h"

namespace Galaxy3D
{
	std::list<Camera *> Camera::m_cameras;
	std::shared_ptr<Camera> Camera::m_current;
    std::shared_ptr<RenderTexture> Camera::m_hdr_render_target;
    std::shared_ptr<RenderTexture> Camera::m_image_effect_buffer;
    std::shared_ptr<RenderTexture> Camera::m_image_effect_buffer_back;

	Camera::Camera():
		m_clear_flags(CameraClearFlags::SolidColor),
		m_clear_color(0, 0, 1, 1),
		m_depth(0),
		m_culling_mask(-1),
		m_orthographic(false),
		m_orthographic_size(1),
		m_field_of_view(60),
		m_near_clip_plane(0.3f),
		m_far_clip_plane(1000),
		m_rect(0, 0, 1, 1),
        m_hdr(false)
	{
		m_cameras.push_back(this);
		m_cameras.sort(Less);
	}

	void Camera::Start()
	{
		UpdateMatrix();
	}

	Camera::~Camera()
	{
		m_cameras.remove(this);
	}

	bool Camera::Less(const Camera *c1, const Camera *c2)
	{
		return c1->m_depth < c2->m_depth;
	}

	void Camera::SetDepth(int depth)
	{
		m_depth = depth;
		m_cameras.sort(Less);
	}

	void Camera::UpdateMatrix()
	{
		int width = Screen::GetWidth();
		int height = Screen::GetHeight();
		auto transform = GetTransform();

		m_view_matrix = Matrix4x4::LookTo(
			transform->GetPosition(),
			transform->GetForward(),
			transform->GetUp());
		
		if(!m_orthographic)
		{
			m_projection_matrix = Matrix4x4::Perspective(m_field_of_view, width / (float) height, m_near_clip_plane, m_far_clip_plane);
		}
		else
		{
			float top = m_orthographic_size;
			float bottom = -m_orthographic_size;
			float plane_h = m_orthographic_size * 2;
			float plane_w = plane_h * (width * m_rect.width) / (height * m_rect.height);
			m_projection_matrix = Matrix4x4::Ortho(-plane_w/2, plane_w/2, bottom, top, m_near_clip_plane, m_far_clip_plane);
		}

		m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	}

	void Camera::SetViewport(int w, int h) const
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		D3D11_VIEWPORT vp;
		vp.Width = m_rect.width * w;
		vp.Height = m_rect.height * h;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = m_rect.left * w;
		vp.TopLeftY = m_rect.top * h;

		context->RSSetViewports(1, &vp);
	}

	void Camera::UpdateTime()
	{
		if(GTTime::m_time_record < 0)
		{
			GTTime::m_time_record = GTTime::GetRealTimeSinceStartup();
			GTTime::m_frame_record = GTTime::GetFrameCount();
		}

		float now = GTTime::GetRealTimeSinceStartup();
		int frame = GTTime::GetFrameCount();
		if(now - GTTime::m_time_record > 1)
		{
			GTTime::m_fps = frame - GTTime::m_frame_record;
			GTTime::m_time_record = now;
			GTTime::m_frame_record = frame;

			//Debug::Log("fps:%03d render:%.3f update:%.3f dc:%d", GTTime::m_fps, GTTime::m_render_time * 1000, GTTime::m_update_time * 1000, GTTime::m_draw_call);
		}

		GTTime::m_frame_count++;
	}

	void Camera::RenderAll()
	{
		float time = GTTime::GetRealTimeSinceStartup();

        GTTime::m_draw_call = 0;

        for(auto i : m_cameras)
        {
            auto obj = i->GetGameObject();
            if(obj->IsActiveInHierarchy() && i->IsEnable())
            {
                m_current = std::dynamic_pointer_cast<Camera>(i->GetComponentPtr());
                i->Render();

                auto effects = obj->GetComponents<ImageEffect>();
                for(size_t j=0; j<effects.size(); j++)
                {
                    std::shared_ptr<RenderTexture> source;
                    std::shared_ptr<RenderTexture> dest;

                    if(i->m_hdr && j == 0)
                    {
                        source = m_hdr_render_target;
                    }
                    else
                    {
                        source = m_image_effect_buffer;
                    }

                    if(j == effects.size() - 1)
                    {
                        if(i->m_render_texture)
                        {
                            dest = i->m_render_texture;
                        }
                        else
                        {
                            dest = GraphicsDevice::GetInstance()->GetScreenBuffer();
                        }
                    }
                    else
                    {
                        dest = m_image_effect_buffer_back;
                    }

                    effects[j]->OnRenderImage(source, dest);
                    std::swap(m_image_effect_buffer, m_image_effect_buffer_back);
                }

                // have hdr but no image effect, blit hdr buffer to target directly
                if(effects.empty() && i->m_hdr)
                {
                    std::shared_ptr<RenderTexture> dest;
                    if(i->m_render_texture)
                    {
                        dest = i->m_render_texture;
                    }
                    else
                    {
                        dest = GraphicsDevice::GetInstance()->GetScreenBuffer();
                    }

                    GraphicsDevice::GetInstance()->Blit(m_hdr_render_target, dest, std::shared_ptr<Material>(), 0);
                }

                m_current.reset();
            }
        }

		GraphicsDevice::GetInstance()->GetSwapChain()->Present(0, 0);

		GTTime::m_render_time = GTTime::GetRealTimeSinceStartup() - time;

		UpdateTime();
	}

	void Camera::Render()
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto effects = GetGameObject()->GetComponents<ImageEffect>();
        std::shared_ptr<RenderTexture> render_target;
        int width, height;

        if(m_render_texture)
        {
            render_target = m_render_texture;
        }
        else
        {
            render_target = GraphicsDevice::GetInstance()->GetScreenBuffer();
        }

        width = render_target->GetWidth();
        height = render_target->GetHeight();

        if(!effects.empty())
        {
            CreateImageEffectBufferIfNeeded(width, height);
        }

        if(m_hdr)
        {
            CreateHDRTargetIfNeeded(width, height);

            render_target = m_hdr_render_target;
        }
        else
        {
            if(!effects.empty())
            {
                render_target = m_image_effect_buffer;
            }
        }

        SetRenderTarget(render_target);
		
		Renderer::RenderAll();
	}

    void Camera::SetRenderTarget(const std::shared_ptr<RenderTexture> &render_texture)
    {
        int width = render_texture->GetWidth();
        int height = render_texture->GetHeight();

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto color_buffer = render_texture->GetRenderTargetView();
        auto depth_buffer = render_texture->GetDepthStencilView();

        context->OMSetRenderTargets(1, &color_buffer, depth_buffer);
        SetViewport(width, height);

        m_render_target_binding = render_texture;

        Clear();
    }

    void Camera::Clear()
    {
        if(m_render_target_binding->IsKeepBuffer())
        {
            return;
        }

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto color_buffer = m_render_target_binding->GetRenderTargetView();
        auto depth_buffer = m_render_target_binding->GetDepthStencilView();

        if(m_clear_flags == CameraClearFlags::SolidColor)
        {
            context->ClearRenderTargetView(color_buffer, (const float *) &m_clear_color);

            if(depth_buffer != 0)
            {
                context->ClearDepthStencilView(depth_buffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            }
        }
        else if(m_clear_flags == CameraClearFlags::Depth)
        {
            if(depth_buffer != 0)
            {
                context->ClearDepthStencilView(depth_buffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            }
        }
    }

    void Camera::Done()
    {
        m_hdr_render_target.reset();
        m_image_effect_buffer.reset();
        m_image_effect_buffer_back.reset();
    }

    void Camera::CreateHDRTargetIfNeeded(int w, int h)
    {
        if(!m_hdr_render_target)
        {
            m_hdr_render_target = RenderTexture::Create(w, h, RenderTextureFormat::RGBAFloat, DepthBuffer::Depth_24);
        }
    }

    void Camera::CreateImageEffectBufferIfNeeded(int w, int h)
    {
        if(!m_image_effect_buffer)
        {
            m_image_effect_buffer = RenderTexture::Create(w, h, RenderTextureFormat::RGBA32, DepthBuffer::Depth_24);
        }

        if(!m_image_effect_buffer_back)
        {
            m_image_effect_buffer_back = RenderTexture::Create(w, h, RenderTextureFormat::RGBA32, DepthBuffer::Depth_24);
        }
    }

    Vector3 Camera::ScreenToViewportPoint(const Vector3 &position)
    {
        float x = position.x / Screen::GetWidth();
        x = (x - m_rect.left) / m_rect.width;

        float y = position.y / Screen::GetHeight();
        y = (y - m_rect.top) / m_rect.height;

        return Vector3(x, y, 0);
    }

    Vector3 Camera::ViewportToWorldPoint(const Vector3 &position)
    {
        Vector3 pos;

        if(m_orthographic)
        {
            float plane_h = m_orthographic_size * 2;
            float plane_w = plane_h * (Screen::GetWidth() * m_rect.width) / (Screen::GetHeight() * m_rect.height);
            float vp_x = (position.x - 0.5f) * plane_w;
            float vp_y = (position.y - 0.5f) * plane_h;

            pos = Vector3(vp_x, vp_y, position.z);
            pos = GetTransform()->TransformPoint(pos);
        }
        else
        {
            float plane_h = 2 * m_near_clip_plane * tan(m_field_of_view * Mathf::Deg2Rad / 2);
            float plane_w = plane_h * (Screen::GetWidth() * m_rect.width) / (Screen::GetHeight() * m_rect.height);
            float vp_x = (position.x - 0.5f) * plane_w;
            float vp_y = (position.y - 0.5f) * plane_h;
            float vp_z = m_near_clip_plane;

            float x = vp_x / vp_z * position.z;
            float y = vp_y / vp_z * position.z;
            float z = position.z;
            pos = Vector3(x , y , z);
            pos = GetTransform()->TransformPoint(pos);
        }

        return pos;
    }

    Vector3 Camera::WorldToViewportPoint(const Vector3 &position)
    {
        Vector3 pos = position - GetTransform()->GetPosition();
        pos = Quaternion::Inverse(GetTransform()->GetRotation()) * pos;

        if(m_orthographic)
        {
            float plane_h = m_orthographic_size * 2;
            float plane_w = plane_h * (Screen::GetWidth() * m_rect.width) / (Screen::GetHeight() * m_rect.height);

            float z = pos.z;
            float y = pos.y / plane_h + 0.5f;
            float x = pos.x / plane_w + 0.5f;

            pos = Vector3(x, y, z);
        }
        else
        {
            float plane_h = 2 * m_near_clip_plane * tan(m_field_of_view * Mathf::Deg2Rad / 2);
            float plane_w = plane_h * (Screen::GetWidth() * m_rect.width) / (Screen::GetHeight() * m_rect.height);

            float z = pos.z;
            float y = pos.y / z * m_near_clip_plane / plane_h + 0.5f;
            float x = pos.x / z * m_near_clip_plane / plane_w + 0.5f;

            pos = Vector3(x, y, z);
        }

        return pos;
    }

    Ray Camera::ScreenPointToRay(const Vector3 &position)
    {
        Vector3 vp = ScreenToViewportPoint(position);

        if(m_orthographic)
        {
            float plane_h = m_orthographic_size * 2;
            float plane_w = plane_h * (Screen::GetWidth() * m_rect.width) / (Screen::GetHeight() * m_rect.height);
            float vp_x = (vp.x - 0.5f) * plane_w;
            float vp_y = (vp.y - 0.5f) * plane_h;

            Vector3 dir = Vector3(0, 0, 1);
            dir = GetTransform()->TransformDirection(dir);
            Vector3 origin = Vector3(vp_x, vp_y, m_near_clip_plane);
            origin = GetTransform()->TransformPoint(origin);

            return Ray(origin, dir);
        }
        else
        {
            float plane_h = 2 * m_near_clip_plane * tan(m_field_of_view * Mathf::Deg2Rad / 2);
            float plane_w = plane_h * (Screen::GetWidth() * m_rect.width) / (Screen::GetHeight() * m_rect.height);
            float vp_x = (vp.x - 0.5f) * plane_w;
            float vp_y = (vp.y - 0.5f) * plane_h;
            float vp_z = m_near_clip_plane;

            Vector3 dir = Vector3(vp_x, vp_y, vp_z);
            dir = GetTransform()->TransformDirection(dir);
            Vector3 origin = GetTransform()->GetPosition();

            return Ray(origin, dir);
        }
    }
}
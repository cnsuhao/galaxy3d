#include "Camera.h"
#include "GameObject.h"
#include "GTTime.h"
#include "Debug.h"
#include "Screen.h"
#include "Renderer.h"
#include "RenderTexture.h"
#include "ImageEffect.h"
#include "ImageEffectToneMapping.h"
#include "RenderSettings.h"
#include "Light.h"
#include "LayerMask.h"
#include "SkyBox.h"

namespace Galaxy3D
{
	std::list<Camera *> Camera::m_cameras;
	std::shared_ptr<Camera> Camera::m_current;
    std::shared_ptr<RenderTexture> Camera::m_hdr_render_target;
    std::shared_ptr<RenderTexture> Camera::m_hdr_render_target_back;
    std::shared_ptr<RenderTexture> Camera::m_image_effect_buffer;
    std::shared_ptr<RenderTexture> Camera::m_image_effect_buffer_back;
    std::shared_ptr<RenderTexture> Camera::m_g_buffer[G_BUFFER_MRT_COUNT];
    std::shared_ptr<Material> Camera::m_deferred_shading_mat;

    void Camera::Done()
    {
        m_hdr_render_target.reset();
        m_hdr_render_target_back.reset();
        m_image_effect_buffer.reset();
        m_image_effect_buffer_back.reset();
        m_deferred_shading_mat.reset();
        for(auto &i : m_g_buffer)
        {
            i.reset();
        }
    }

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
        m_hdr(false),
        m_deferred_shading(false),
        m_transform_changed(true)
	{
		m_cameras.push_back(this);
		m_cameras.sort(Less);
	}

    void Camera::OnTranformChanged()
    {
        m_transform_changed = true;
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

    void Camera::SetViewport(const Rect &rect)
    {
        GraphicsDevice::GetInstance()->SetViewport((int) rect.left, (int) rect.top, (int) rect.width, (int) rect.height);
    }

	void Camera::SetViewport(int w, int h) const
	{
        GraphicsDevice::GetInstance()->SetViewport((int) (m_rect.left * w), (int) (m_rect.top * h), (int) (m_rect.width * w), (int) (m_rect.height * h));
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

    void Camera::ImageEffectsOpaque()
    {
        std::shared_ptr<RenderTexture> hdr_render_target_front = m_hdr_render_target;
        std::shared_ptr<RenderTexture> image_effect_buffer_front = m_image_effect_buffer;
        auto effects = GetGameObject()->GetComponents<ImageEffect>();

        // remove disabled or default effects
        for(size_t i=0; i<effects.size(); )
        {
            if(!effects[i]->IsEnable() || !effects[i]->IsOpaque())
            {
                effects.erase(effects.begin() + i);
            }
            else
            {
                i++;
            }
        }

        if(effects.empty())
        {
            return;
        }

        if(hdr_render_target_front)
        {
            hdr_render_target_front->MarkKeepBuffer(true);
        }
        if(image_effect_buffer_front)
        {
            image_effect_buffer_front->MarkKeepBuffer(true);
        }

        for(size_t i=0; i<effects.size(); i++)
        {
            std::shared_ptr<RenderTexture> source;
            std::shared_ptr<RenderTexture> dest;

            if(m_hdr)
            {
                source = m_hdr_render_target;
                dest = m_hdr_render_target_back;
            }
            else
            {
                source = m_image_effect_buffer;
                dest = m_image_effect_buffer_back;
            }

            effects[i]->OnRenderImage(source, dest);

            if(m_hdr)
            {
                std::swap(m_hdr_render_target, m_hdr_render_target_back);
            }
            else
            {
                std::swap(m_image_effect_buffer, m_image_effect_buffer_back);
            }
        }

        // make sure front buffer in front, blit and swap if not
        if(m_hdr)
        {
            if(hdr_render_target_front != m_hdr_render_target)
            {
                GraphicsDevice::GetInstance()->Blit(m_hdr_render_target, m_hdr_render_target_back, std::shared_ptr<Material>(), 0);
                std::swap(m_hdr_render_target, m_hdr_render_target_back);
            }
        }
        else
        {
            if(image_effect_buffer_front != m_image_effect_buffer)
            {
                GraphicsDevice::GetInstance()->Blit(m_image_effect_buffer, m_image_effect_buffer_back, std::shared_ptr<Material>(), 0);
                std::swap(m_image_effect_buffer, m_image_effect_buffer_back);
            }
        }
        
        if(hdr_render_target_front)
        {
            hdr_render_target_front->MarkKeepBuffer(false);
        }
        if(image_effect_buffer_front)
        {
            image_effect_buffer_front->MarkKeepBuffer(false);
        }
    }

    void Camera::ImageEffectsDefault()
    {
        std::shared_ptr<RenderTexture> hdr_render_target_front = m_hdr_render_target;
        std::shared_ptr<RenderTexture> image_effect_buffer_front = m_image_effect_buffer;
        bool hdr = m_hdr;
        auto effects = GetGameObject()->GetComponents<ImageEffect>();
        bool use_effect = !effects.empty();
        
        // remove disabled or opaque effects
        for(size_t i=0; i<effects.size(); )
        {
            if(!effects[i]->IsEnable() || effects[i]->IsOpaque())
            {
                effects.erase(effects.begin() + i);
            }
            else
            {
                i++;
            }
        }

        for(size_t i=0; i<effects.size(); i++)
        {
            std::shared_ptr<RenderTexture> source;
            std::shared_ptr<RenderTexture> dest;

            if(hdr)
            {
                source = m_hdr_render_target;
            }
            else
            {
                source = m_image_effect_buffer;
            }

            if(i == effects.size() - 1)
            {
                if(m_render_texture)
                {
                    dest = m_render_texture;
                }
                else
                {
                    dest = GraphicsDevice::GetInstance()->GetScreenBuffer();
                }
            }
            else
            {
                auto tone_mapping = std::dynamic_pointer_cast<ImageEffectToneMapping>(effects[i]);
                if(tone_mapping)
                {
                    hdr = false;
                }

                if(hdr)
                {
                    dest = m_hdr_render_target_back;
                }
                else
                {
                    dest = m_image_effect_buffer_back;
                }
            }

            effects[i]->OnRenderImage(source, dest);

            std::swap(m_hdr_render_target, m_hdr_render_target_back);
            std::swap(m_image_effect_buffer, m_image_effect_buffer_back);
        }

        if(effects.empty() && (use_effect || m_deferred_shading))
        {
            std::shared_ptr<RenderTexture> dest;
            if(m_render_texture)
            {
                dest = m_render_texture;
            }
            else
            {
                dest = GraphicsDevice::GetInstance()->GetScreenBuffer();
            }

            if(m_hdr)
            {
                GraphicsDevice::GetInstance()->Blit(m_hdr_render_target, dest, std::shared_ptr<Material>(), 0);
            }
            else
            {
                GraphicsDevice::GetInstance()->Blit(m_image_effect_buffer, dest, std::shared_ptr<Material>(), 0);
            }
        }

        // restore front buffer to front
        if(hdr_render_target_front != m_hdr_render_target)
        {
            std::swap(m_hdr_render_target, m_hdr_render_target_back);
        }
        if(image_effect_buffer_front != m_image_effect_buffer)
        {
            std::swap(m_image_effect_buffer, m_image_effect_buffer_back);
        }
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

                if(i->m_transform_changed)
                {
                    i->m_transform_changed = false;
                    i->UpdateMatrix();
                }

                Renderer::Prepare();
                i->RenderShadowMaps();
                i->Render();

                m_current.reset();
            }
        }

		GraphicsDevice::GetInstance()->Present();

		GTTime::m_render_time = GTTime::GetRealTimeSinceStartup() - time;

		UpdateTime();
	}

    bool Camera::IsCulling(std::shared_ptr<GameObject> &obj) const
    {
        return (m_culling_mask & LayerMask::GetMask(obj->GetLayer())) == 0;
    }

    void Camera::RenderShadowMaps()
    {
        auto shadow_lights = Light::GetLightsHasShadow();

        for(auto i : shadow_lights)
        {
            if(!IsCulling(i->GetGameObject()))
            {
                RenderSettings::SetLightRenderingShadowMap(std::dynamic_pointer_cast<Light>(i->GetComponentPtr()));

                // set target to shadow map,
                // build light matrix
                i->PrepareForRenderShadowMap();
                if(i->GetType() == LightType::Directional && i->IsCascade())
                {
                    for(int j=0; j<Light::CASCADE_SHADOW_COUNT; j++)
                    {
                        i->SetCascadeViewport(j);
                        Renderer::RenderOpaqueGeometry();
                    }
                }
                else
                {
                    Renderer::RenderOpaqueGeometry();
                }
                
                RenderSettings::SetLightRenderingShadowMap(std::shared_ptr<Light>());
            }
        }
    }

	void Camera::Render()
	{
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

        if(!effects.empty() || (!m_hdr && m_deferred_shading))
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
            if(!effects.empty() || m_deferred_shading)
            {
                render_target = m_image_effect_buffer;
            }
        }

        if(m_deferred_shading)
        {
            CreateDeferredShadingResourcesIfNeeded(width, height);

            SetGBufferTarget(render_target);
        }
        else
        {
            SetRenderTarget(render_target);
        }
		
        Renderer::RenderOpaqueGeometry();

        if(m_deferred_shading)
        {
            DeferredShading();
        }

        RenderSkyBox();

        ImageEffectsOpaque();
        Renderer::RenderTransparentGeometry();
        ImageEffectsDefault();
	}

    void Camera::RenderSkyBox()
    {
        auto sky = GetGameObject()->GetComponent<SkyBox>();
        if(sky)
        {
            sky->Render();
        }
    }

    void Camera::DeferredShading()
    {
        std::shared_ptr<RenderTexture> front;
        std::shared_ptr<RenderTexture> back;

        if(m_hdr)
        {
            front = m_hdr_render_target;
            back = m_hdr_render_target_back;
        }
        else
        {
            front = m_image_effect_buffer;
            back = m_image_effect_buffer_back;
        }

        // swap diffuse buffer to back
        RenderTexture::SwapColorBuffer(front, back);

        // now target is front, and diffuse is in back,
        // add all lights colors to front buffer
        m_deferred_shading_mat->SetColor("GlobalAmbient", RenderSettings::light_ambient);
        m_deferred_shading_mat->SetMainTexture(back);
        m_deferred_shading_mat->SetTexture("_GBuffer1", m_g_buffer[0]);
        m_deferred_shading_mat->SetTexture("_GBuffer2", m_g_buffer[1]);
        m_deferred_shading_mat->SetTexture("_GBuffer3", m_g_buffer[2]);
        m_deferred_shading_mat->SetVector("EyePosition", Vector4(GetTransform()->GetPosition()));
        m_deferred_shading_mat->SetMatrix("InvViewProjection", GetViewProjectionMatrix().Inverse());

        // set target to front force
        front->MarkKeepBuffer(true);
        SetRenderTarget(front, true);
        front->MarkKeepBuffer(false);

        Light::DeferredShadingLights(m_deferred_shading_mat);
    }

    void Camera::SetGBufferTarget(std::shared_ptr<RenderTexture> &render_texture)
    {
        int width = render_texture->GetWidth();
        int height = render_texture->GetHeight();

        std::vector<std::shared_ptr<RenderTexture>> color_buffers(G_BUFFER_MRT_COUNT + 1);
        color_buffers[0] = render_texture;
        color_buffers[1] = m_g_buffer[0];
        color_buffers[2] = m_g_buffer[1];
        color_buffers[3] = m_g_buffer[2];

        GraphicsDevice::GetInstance()->SetRenderTargets(color_buffers, render_texture);
        SetViewport(width, height);

        m_render_target_binding = render_texture;

        GraphicsDevice::GetInstance()->ClearRenderTarget(CameraClearFlags::SolidColor, m_clear_color, 1.0f, 0);
    }

    void Camera::SetRenderTarget(const std::shared_ptr<RenderTexture> &render_texture, bool force)
    {
        if(!force && m_render_target_binding == render_texture)
        {
            return;
        }

        int width = render_texture->GetWidth();
        int height = render_texture->GetHeight();

        std::vector<std::shared_ptr<RenderTexture>> color_buffers(1);
        color_buffers[0] = render_texture;

        GraphicsDevice::GetInstance()->SetRenderTargets(color_buffers, render_texture);
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

        if(m_clear_flags == CameraClearFlags::SolidColor || m_clear_flags == CameraClearFlags::Depth)
        {
            GraphicsDevice::GetInstance()->ClearRenderTarget(m_clear_flags, m_clear_color, 1.0f, 0);
        }
    }

    int Camera::GetPixelWidth() const
    {
        std::shared_ptr<RenderTexture> target;

        if(m_render_texture)
        {
            target = m_render_texture;
        }
        else
        {
            target = GraphicsDevice::GetInstance()->GetScreenBuffer();
        }

        return target->GetWidth();
    }

    int Camera::GetPixelHeight() const
    {
        std::shared_ptr<RenderTexture> target;

        if(m_render_texture)
        {
            target = m_render_texture;
        }
        else
        {
            target = GraphicsDevice::GetInstance()->GetScreenBuffer();
        }

        return target->GetHeight();
    }

    float Camera::GetAspect() const
    {
        std::shared_ptr<RenderTexture> target;

        if(m_render_texture)
        {
            target = m_render_texture;
        }
        else
        {
            target = GraphicsDevice::GetInstance()->GetScreenBuffer();
        }

        return target->GetWidth() / (float) target->GetHeight();
    }

    std::shared_ptr<RenderTexture> Camera::GetDepthTexture() const
    {
        auto effects = GetGameObject()->GetComponents<ImageEffect>();
        std::shared_ptr<RenderTexture> depth_texture;

        if(m_render_texture)
        {
            depth_texture = m_render_texture;
        }
        else
        {
            depth_texture = GraphicsDevice::GetInstance()->GetScreenBuffer();
        }
        
        if(m_hdr)
        {
            if(m_hdr_render_target->GetDepthStencilView() != NULL)
            {
                depth_texture = m_hdr_render_target;
            }
            else
            {
                depth_texture = m_hdr_render_target_back;
            }
        }
        else
        {
            if(!effects.empty())
            {
                if(m_image_effect_buffer->GetDepthStencilView() != NULL)
                {
                    depth_texture = m_image_effect_buffer;
                }
                else
                {
                    depth_texture = m_image_effect_buffer_back;
                }
            }
        }

        return depth_texture;
    }

    void Camera::CreateHDRTargetIfNeeded(int w, int h)
    {
        if(!m_hdr_render_target)
        {
            m_hdr_render_target = RenderTexture::Create(w, h, RenderTextureFormat::RGBAHalf, DepthBuffer::Depth_24);
        }

        if(!m_hdr_render_target_back)
        {
            m_hdr_render_target_back = RenderTexture::Create(w, h, RenderTextureFormat::RGBAHalf, DepthBuffer::Depth_0);
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
            m_image_effect_buffer_back = RenderTexture::Create(w, h, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0);
        }
    }

    void Camera::CreateDeferredShadingResourcesIfNeeded(int w, int h)
    {
        // 32bit RGHalf for normal 
        if(!m_g_buffer[0])
        {
            m_g_buffer[0] = RenderTexture::Create(w, h, RenderTextureFormat::RGHalf, DepthBuffer::Depth_0);
        }

        // 32bit RGBA32 for motion vector and specular 
        if(!m_g_buffer[1])
        {
            m_g_buffer[1] = RenderTexture::Create(w, h, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0);
        }

        // 32bit RFloat for depth 
        if(!m_g_buffer[2])
        {
            m_g_buffer[2] = RenderTexture::Create(w, h, RenderTextureFormat::RFloat, DepthBuffer::Depth_0);
        }

        if(!m_deferred_shading_mat)
        {
            m_deferred_shading_mat = Material::Create("DeferredShading");
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
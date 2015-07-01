#include "Camera.h"
#include "GameObject.h"
#include "GraphicsDevice.h"
#include "GTTime.h"
#include "Debug.h"
#include "Screen.h"
#include "Renderer.h"

namespace Galaxy3D
{
	std::list<Camera *> Camera::m_cameras;
	std::shared_ptr<Camera> Camera::m_current;

	Camera::Camera():
		m_clear_flags(CameraClearFlags::SolidColor),
		m_clear_color(0, 0, 1, 1),
		m_depth(0),
		m_culling_mask(-1),
		m_orthographic(true),
		m_orthographic_size(1),
		m_field_of_view(60),
		m_near_clip_plane(-1),//0.3f
		m_far_clip_plane(1),//1000
		m_rect(0, 0, 1, 1)
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

	void Camera::SetViewport() const
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		int width = Screen::GetWidth();
		int height = Screen::GetHeight();

		D3D11_VIEWPORT vp;
		vp.Width = m_rect.width * width;
		vp.Height = m_rect.height * height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = m_rect.left * width;
		vp.TopLeftY = m_rect.top * height;

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

			Debug::Log("fps:%d render:%f update:%f", GTTime::m_fps, GTTime::m_render_time * 1000, GTTime::m_update_time * 1000);
		}

		GTTime::m_frame_count++;
	}

	void Camera::RenderAll()
	{
		float time = GTTime::GetRealTimeSinceStartup();

		UpdateTime();

		for(auto i : m_cameras)
		{
			auto obj = i->GetGameObject();
			if(obj->IsActiveInHierarchy() && i->IsEnable())
			{
				i->Render();
			}
		}

		GraphicsDevice::GetInstance()->GetSwapChain()->Present(0, 0);

		GTTime::m_render_time = GTTime::GetRealTimeSinceStartup() - time;
	}

	void Camera::Render() const
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
		auto render_buffer= GraphicsDevice::GetInstance()->GetRenderTargetView();
		auto depth_buffer = GraphicsDevice::GetInstance()->GetDepthStencilView();

		SetViewport();

		//set target
		context->OMSetRenderTargets(1, &render_buffer, depth_buffer);

		//clear
		context->ClearRenderTargetView(render_buffer, (const float *) &m_clear_color);
		context->ClearDepthStencilView(depth_buffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		//render
		m_current = GetGameObject()->GetComponent<Camera>();
		Renderer::RenderAll();
	}
}
#ifndef __Camera_h__
#define __Camera_h__

#include "Component.h"
#include "CameraClearFlags.h"
#include "Color.h"
#include "Matrix4x4.h"
#include "Rect.h"
#include "GraphicsDevice.h"
#include <list>

namespace Galaxy3D
{
    class RenderTexture;

	class Camera : public Component
	{
	public:
		static void RenderAll();
		static std::shared_ptr<Camera> GetCurrent() {return m_current;}
        static void Done();
		Camera();
		virtual ~Camera();
		void SetClearFlags(CameraClearFlags::Enum flag) {m_clear_flags = flag;}
		void SetClearColor(const Color &color) {m_clear_color = color;}
		void SetDepth(int depth);
		void SetCullingMask(int mask) {m_culling_mask = mask;}
		int GetCullingMask() const {return m_culling_mask;}
		void SetOrthographic(bool ortho) {m_orthographic = ortho;}
		void SetOrthographicSize(float ortho_size) {m_orthographic_size = ortho_size;}
		void SetFieldOfView(float fov) {m_field_of_view = fov;}
		void SetClipPlane(float near_clip, float far_clip) {m_near_clip_plane = near_clip; m_far_clip_plane = far_clip;}
		void SetRect(const Rect &rect) {m_rect = rect;}
		void UpdateMatrix();
		const Matrix4x4 &GetViewProjectionMatrix() const {return m_view_projection_matrix;}
        Vector3 ScreenToViewportPoint(const Vector3 &position);
        Vector3 ViewportToWorldPoint(const Vector3 &position);
        Vector3 WorldToViewportPoint(const Vector3 &position);
        Ray ScreenPointToRay(const Vector3 &position);
        void EnableHDR(bool enable) {m_hdr = enable;}
        void SetRenderTarget(const std::shared_ptr<RenderTexture> &render_target) {m_render_target = render_target;}

	protected:
		virtual void Start();
		
	private:
		static std::list<Camera *> m_cameras;
		static std::shared_ptr<Camera> m_current;
        static std::shared_ptr<RenderTexture> m_hdr_render_target;
        static std::shared_ptr<RenderTexture> m_image_effect_buffer;
        static std::shared_ptr<RenderTexture> m_image_effect_buffer_back;
		CameraClearFlags::Enum m_clear_flags;
		Color m_clear_color;
		int m_depth;
		int m_culling_mask;
		bool m_orthographic;
		float m_orthographic_size;
		float m_field_of_view;
		float m_near_clip_plane;
		float m_far_clip_plane;
		Rect m_rect;
        bool m_hdr;
        std::shared_ptr<RenderTexture> m_render_target;
		Matrix4x4 m_view_matrix;
		Matrix4x4 m_projection_matrix;
		Matrix4x4 m_view_projection_matrix;

		static bool Camera::Less(const Camera *c1, const Camera *c2);
		static void UpdateTime();
        static void CreateHDRTargetIfNeeded(int w, int h);
        static void CreateImageEffectBufferIfNeeded(int w, int h);
		void Render() const;
		void SetViewport(int w, int h) const;
        ID3D11RenderTargetView *GetRenderTargetColorBuffer() const;
        ID3D11DepthStencilView *GetRenderTargetDepthBuffer() const;
        void GetRenderTargetSize(int *w, int *h) const;
	};
}

#endif
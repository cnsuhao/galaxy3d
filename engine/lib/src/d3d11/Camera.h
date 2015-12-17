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
        float GetFieldOfView() const {return m_field_of_view;}
		void SetClipPlane(float near_clip, float far_clip) {m_near_clip_plane = near_clip; m_far_clip_plane = far_clip;}
        float GetClipNear() const {return m_near_clip_plane;}
        float GetClipFar() const {return m_far_clip_plane;}
        void SetRect(const Rect &rect) {m_rect = rect;}
		void UpdateMatrix();
		const Matrix4x4 &GetViewProjectionMatrix() const {return m_view_projection_matrix;}
        Vector3 ScreenToViewportPoint(const Vector3 &position);
        Vector3 ViewportToWorldPoint(const Vector3 &position);
        Vector3 WorldToViewportPoint(const Vector3 &position);
        Ray ScreenPointToRay(const Vector3 &position);
        void EnableHDR(bool enable) {m_hdr = enable;}
        void EnableDeferredShading(bool enable) {m_deferred_shading = enable;}
        bool IsDeferredShading() const {return m_deferred_shading;}
        void SetRenderTexture(const std::shared_ptr<RenderTexture> &render_texture) {m_render_texture = render_texture;}
        void SetRenderTarget(const std::shared_ptr<RenderTexture> &render_texture);
        std::shared_ptr<RenderTexture> GetRenderTarget() const {return m_render_target_binding;}
        std::shared_ptr<RenderTexture> GetDepthTexture() const;

	protected:
		virtual void Start();
		
	private:
        static const int G_BUFFER_MRT_COUNT = 2;
		static std::list<Camera *> m_cameras;
		static std::shared_ptr<Camera> m_current;
        static std::shared_ptr<RenderTexture> m_hdr_render_target;
        static std::shared_ptr<RenderTexture> m_hdr_render_target_back;
        static std::shared_ptr<RenderTexture> m_image_effect_buffer;
        static std::shared_ptr<RenderTexture> m_image_effect_buffer_back;
        static std::shared_ptr<RenderTexture> m_g_buffer[G_BUFFER_MRT_COUNT];
        static std::shared_ptr<Material> m_deferred_shading_mat;
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
        bool m_deferred_shading;
        std::shared_ptr<RenderTexture> m_render_texture;
        std::shared_ptr<RenderTexture> m_render_target_binding;
		Matrix4x4 m_view_matrix;
		Matrix4x4 m_projection_matrix;
		Matrix4x4 m_view_projection_matrix;

		static bool Camera::Less(const Camera *c1, const Camera *c2);
		static void UpdateTime();
        static void CreateHDRTargetIfNeeded(int w, int h);
        static void CreateImageEffectBufferIfNeeded(int w, int h);
        static void CreateDeferredShadingResourcesIfNeeded(int w, int h);
		void Render();
		void SetViewport(int w, int h) const;
        void Clear();
        //
        // 摘要:
        //     ///
        //     Must keep depth buffer dont clear in this procedure
        //     ///
        void ImageEffectsOpaque();
        void ImageEffectsDefault();
        void SetGBufferTarget(std::shared_ptr<RenderTexture> &render_texture);
        void DeferredShading();
        void DeferredShadingLightGlobalDirectional(std::shared_ptr<RenderTexture> &front, std::shared_ptr<RenderTexture> &back);
	};
}

#endif
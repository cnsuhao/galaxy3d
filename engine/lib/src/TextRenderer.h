#ifndef __TextRenderer_h__
#define __TextRenderer_h__

#include "Renderer.h"
#include "Label.h"

namespace Galaxy3D
{
    class SpriteBatchRenderer;

	class TextRenderer : public Renderer
	{
	public:
		TextRenderer();
		virtual ~TextRenderer();
		void SetLabel(const std::shared_ptr<Label> &label);
		std::shared_ptr<Label> GetLabel() const {return m_label;}
        void SetAnchor(const Vector4 &anchor);
		std::shared_ptr<Vector4> GetAnchor() const {return m_anchor;}
        void SetColor(const Color &color) {m_color = color;}
        Color GetColor() const {return m_color;}
        void SetClip(bool clip) {m_clip = clip;}
        bool IsClip() const {return m_clip;}
        void SetClipRect(const Vector4 &rect) {m_clip_rect = rect;}
        void SetClipSoft(const Vector2 &soft) {m_clip_soft = soft;}
        void SetClipPanel(const std::weak_ptr<SpriteBatchRenderer> &clip_panel) {m_clip_panel = clip_panel;}
        std::weak_ptr<SpriteBatchRenderer> GetClipPanel() const {return m_clip_panel;}

	protected:
        virtual void Start();
		virtual void Render(int material_index);

	private:
		std::shared_ptr<Label> m_label;
        std::shared_ptr<Vector4> m_anchor;
        Color m_color;
		BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;
		int m_vertex_count;
		std::vector<BufferObject> m_vertex_buffer_img;
		std::vector<BufferObject> m_index_buffer_img;
		std::shared_ptr<Material> m_material_img;
        bool m_clip;
        Vector4 m_clip_rect;
        Vector2 m_clip_soft;
        std::weak_ptr<SpriteBatchRenderer> m_clip_panel;
        bool m_dirty;

        void UpdateLabel();
		void CreateVertexBuffer();
		void UpdateVertexBuffer();
		void CreateIndexBuffer();
		void ReleaseBuffer();
		void RenderLabelImage();
		void CreateVertexBufferLabelImage();
		void UpdateVertexBufferLabelImage();
		void CreateIndexBufferLabelImage();
		void ReleaseBufferLabelImage();
	};
}

#endif
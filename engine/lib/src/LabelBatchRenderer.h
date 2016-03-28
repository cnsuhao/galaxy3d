#ifndef __LabelBatchRenderer_h__
#define __LabelBatchRenderer_h__

#include "Renderer.h"
#include "LabelNode.h"

namespace Galaxy3D
{
	class SpriteBatchRenderer;

	class LabelBatchRenderer : public Renderer
	{
	public:
		LabelBatchRenderer();
		virtual ~LabelBatchRenderer();
		void AddLabel(const std::shared_ptr<LabelNode> &label);
		void RemoveLabel(const std::shared_ptr<LabelNode> &label);
		void SetColor(const Color &color) {m_color = color;}
		Color GetColor() const {return m_color;}
        void SetClip(bool clip) {m_clip = clip;}
        bool IsClip() const {return m_clip;}
        void SetClipRect(const Vector4 &rect) {m_clip_rect = rect;}
        Vector4 GetClipRect() const {return m_clip_rect;}
        void SetClipSoft(const Vector2 &soft) {m_clip_soft = soft;}
        Vector2 GetClipSoft() const {return m_clip_soft;}
		void SetClipPanel(const std::weak_ptr<SpriteBatchRenderer> &clip_panel) {m_clip_panel = clip_panel;}
        std::weak_ptr<SpriteBatchRenderer> GetClipPanel() const {return m_clip_panel;}

	protected:
		virtual void Render(int material_index);

	private:
		std::list<std::shared_ptr<LabelNode>> m_labels;
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;
        bool m_clip;
        Vector4 m_clip_rect;
        Vector2 m_clip_soft;
		std::weak_ptr<SpriteBatchRenderer> m_clip_panel;
        bool m_dirty;
		int m_vertex_count_old;
        int m_index_count_old;

		void Release();
		bool IsDirty();
		void UpdateLabels();
		void UpdateVertexBuffer(bool create);
		void UpdateIndexBuffer(bool create);
	};
}

#endif
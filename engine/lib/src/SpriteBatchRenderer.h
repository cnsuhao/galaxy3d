#ifndef __SpriteBatchRenderer_h__
#define __SpriteBatchRenderer_h__

#include "Renderer.h"
#include "SpriteNode.h"
#include <list>

namespace Galaxy3D
{
	class SpriteBatchRenderer : public Renderer
	{
	public:
		SpriteBatchRenderer();
		virtual ~SpriteBatchRenderer();
		void AddSprite(const std::shared_ptr<SpriteNode> &sprite);
		void RemoveSprite(const std::shared_ptr<SpriteNode> &sprite);
		void SetColor(const Color &color) {m_color = color;}
        void SetClip(bool clip) {m_clip = clip;}
        bool IsClip() const {return m_clip;}
        void SetClipRect(const Vector4 &rect) {m_clip_rect = rect;}
        Vector4 GetClipRect() const {return m_clip_rect;}
        void SetClipSoft(const Vector2 &soft) {m_clip_soft = soft;}
        Vector2 GetClipSoft() const {return m_clip_soft;}
        bool IsPointInClipRect(const Vector3 &point);

	protected:
		virtual void Render(int material_index);
		
	private:
		std::list<std::shared_ptr<SpriteNode>> m_sprites;
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;
        bool m_clip;
        Vector4 m_clip_rect;
        Vector2 m_clip_soft;
        bool m_dirty;
		int m_vertex_count_old;
        int m_index_count_old;

        bool IsDirty();
        void UpdateSprites();
		void UpdateVertexBuffer(bool create);
		void UpdateIndexBuffer(bool create);
		void Release();
	};
}

#endif
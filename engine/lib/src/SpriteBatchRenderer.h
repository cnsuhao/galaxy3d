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

	protected:
		virtual void Render(int material_index);
		
	private:
		std::list<std::shared_ptr<SpriteNode>> m_sprites;
        int m_vertex_count_old;
        int m_index_count_old;
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;
        bool m_dirty;

        bool IsDirty();
        void UpdateSprites();
		void CreateVertexBuffer();
		void UpdateVertexBuffer();
		void CreateIndexBuffer();
		void Release();
	};
}

#endif
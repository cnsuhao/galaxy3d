#ifndef __SpriteBatchRenderer_h__
#define __SpriteBatchRenderer_h__

#include "Renderer.h"
#include "SpriteNode.h"
#include <list>

namespace Galaxy3D
{
	//	批量绘制精灵，精灵位置由精灵transform控制，不受renderer影响
	//	不允许添加空的sprite，要置空sprite，请使用RemoveSprite
	//	修改sprite属性后，要调用UpdateSprites更新顶点buffer
	//	SetColor会影响批次内所有sprite
	class SpriteBatchRenderer : public Renderer
	{
	public:
		SpriteBatchRenderer();
		virtual ~SpriteBatchRenderer();
		void AddSprite(const std::shared_ptr<SpriteNode> &sprite);
		void RemoveSprite(const std::shared_ptr<SpriteNode> &sprite);
		void SetColor(const Color &color) {m_color = color;}
		void UpdateSprites();//添加完所有sprite node后，需要手动调用

	protected:
		virtual void Render();
		
	private:
		std::list<std::shared_ptr<SpriteNode>> m_sprites;
		std::list<std::shared_ptr<SpriteNode>> m_sprites_cache;
		Color m_color;
		ID3D11Buffer *m_vertex_buffer;
		ID3D11Buffer *m_index_buffer;

		void CreateVertexBuffer();
		void UpdateVertexBuffer();
		void CreateIndexBuffer();
		void Release();
	};
}

#endif
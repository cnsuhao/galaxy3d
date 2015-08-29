#ifndef __SpriteRenderer_h__
#define __SpriteRenderer_h__

#include "Renderer.h"
#include "Sprite.h"

namespace Galaxy3D
{
	class SpriteRenderer : public Renderer
	{
	public:
		SpriteRenderer();
		void SetColor(const Color &color) {m_color = color;}
		void SetSprite(const std::shared_ptr<Sprite> &sprite);
		std::shared_ptr<Sprite> GetSprite() const {return m_sprite;}

	protected:
		virtual void Render();

	private:
		Color m_color;
		std::shared_ptr<Sprite> m_sprite;
	};
}

#endif
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
        void SetAnchor(const Vector4 &anchor);

	protected:
        virtual void Start();
		virtual void Render(int material_index);

	private:
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		std::shared_ptr<Sprite> m_sprite;
	};
}

#endif
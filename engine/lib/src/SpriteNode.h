#ifndef __SpriteNode_h__
#define __SpriteNode_h__

#include "Component.h"
#include "Sprite.h"
#include "Color.h"

namespace Galaxy3D
{
    class SpriteBatchRenderer;

	class SpriteNode : public Component
	{
	public:
		static bool Less(const std::shared_ptr<SpriteNode> &s1, const std::shared_ptr<SpriteNode> &s2)
		{
			return s1->m_sorting_order < s2->m_sorting_order;
		}
		SpriteNode():
			m_color(1, 1, 1, 1),
			m_sorting_order(0)
		{}
		void SetSprite(const std::shared_ptr<Sprite> &sprite) {m_sprite = sprite;}
		std::shared_ptr<Sprite> GetSprite() const {return m_sprite;}
		void SetColor(const Color &color) {m_color = color;}
		Color GetColor() const {return m_color;}
		void SetSortingOrder(int order) {m_sorting_order = order;}
        void SetAnchor(const Vector4 &anchor);
        void SetBatch(const std::weak_ptr<SpriteBatchRenderer> &batch) {m_batch = batch;}
        std::weak_ptr<SpriteBatchRenderer> GetBatch() const {return m_batch;}
        void AnchorTransform();

	private:
		std::shared_ptr<Sprite> m_sprite;
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		int m_sorting_order;
        std::weak_ptr<SpriteBatchRenderer> m_batch;
	};
}

#endif
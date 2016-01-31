#include "SpriteNode.h"
#include "GameObject.h"
#include "UICanvas.h"

namespace Galaxy3D
{
    void SpriteNode::AnchorTransform()
    {
        auto canvas = GetGameObject()->GetComponentInParent<UICanvas>();

        if(canvas && m_anchor)
        {
            canvas->AnchorTransform(GetTransform(), *m_anchor);
        }
    }

    void SpriteNode::SetSprite(const std::shared_ptr<Sprite> &sprite)
    {
        if(m_sprite != sprite)
        {
            m_sprite = sprite;
            m_dirty = true;
        }
    }

    void SpriteNode::SetColor(const Color &color)
    {
        if(m_color != color)
        {
            m_color = color;
            m_dirty = true;
        }
    }

    void SpriteNode::SetSortingOrder(int order)
    {
        if(m_sorting_order != order)
        {
            m_sorting_order = order;
            m_dirty = true;
        }
    }

    void SpriteNode::SetAnchor(const Vector4 &anchor)
    {
        if(!m_anchor || *m_anchor != anchor)
        {
            m_anchor = std::make_shared<Vector4>(anchor);
            m_dirty = true;
        }
    }
}
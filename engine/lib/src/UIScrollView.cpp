#include "UIScrollView.h"
#include "GameObject.h"
#include "BoxCollider.h"
#include "SpriteNode.h"
#include "UIScrollBar.h"

namespace Galaxy3D
{
    UIScrollView::UIScrollView():
        m_amount(0),
        m_target_size(),
        m_view_size(),
        m_view_pos()
    {
    }

    void UIScrollView::Init()
    {
        auto collider = GetGameObject()->GetComponent<BoxCollider>();
        if(collider)
        {
            m_view_size = collider->GetSize();
            m_view_pos = collider->GetCenter();
        }

        if(scroll_target)
        {
            auto sprite = scroll_target->GetComponent<SpriteNode>();
            if(sprite)
            {
                m_target_size = sprite->GetSprite()->GetSize();
            }
        }

        if(scroll_bar)
        {
            if(m_target_size.y > 0)
            {
                scroll_bar->SetRatio(m_view_size.y / m_target_size.y);
            }
        }
    }

    void UIScrollView::SetAmount(float amount)
    {
        if(!Mathf::FloatEqual(m_amount, amount))
        {
            m_amount = amount;

            float from = 0;
            float to = m_target_size.y - m_view_size.y;
            float y = Mathf::Lerp(from, to, amount, false);

            auto pos = scroll_target->GetTransform()->GetLocalPosition();
            pos.y = y;
            scroll_target->GetTransform()->SetLocalPosition(pos);
        }
    }
}
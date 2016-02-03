#include "UIScrollBar.h"
#include "UIScrollView.h"
#include "Transform.h"
#include "GameObject.h"
#include "UICanvas.h"
#include "BoxCollider.h"
#include "Debug.h"

namespace Galaxy3D
{
    void UIScrollBarThumb::OnDragStart()
    {
        m_down_pos = UICanvas::GetLastPosition();

        if(!scroll_bar.expired())
        {
            scroll_bar.lock()->OnDragSliderThumbStart();
        }
    }

    void UIScrollBarThumb::OnDrag(const Vector3 &delta)
    {
        if(!scroll_bar.expired())
        {
            Vector3 pos = UICanvas::GetLastPosition();
            
            scroll_bar.lock()->OnDragSliderThumb(pos - m_down_pos);
        }
    }

    void UIScrollBar::OnDragSliderThumbStart()
    {
        if(!scroll_view.expired())
        {
            m_down_amount = scroll_view.lock()->GetAmount();
        }
        else
        {
            m_down_amount = 0;
        }
    }

    void UIScrollBar::OnDragSliderThumb(const Vector3 &delta)
    {
        if(background && m_foreground)
        {
            auto back_size = background->GetSprite()->GetSize();
            auto fore_size = m_foreground->GetSprite()->GetSize();

            float from = back_size.y / 2 - fore_size.y / 2;
            float to = -from;

            if(to - from < 0)
            {
                float v = m_down_amount - delta.y / (from - to);
                v = Mathf::Clamp01(v);

                SetAmount(v);

                if(!scroll_view.expired())
                {
                    scroll_view.lock()->SetAmount(v);
                }
            }
        }
    }

    std::shared_ptr<SpriteNode> UIScrollBar::GetForeground()
    {
        if(!m_foreground)
        {
            if(thumb)
            {
                m_foreground = thumb->GetGameObject()->GetComponent<SpriteNode>();
            }
        }

        return m_foreground;
    }

    void UIScrollBar::SetAmount(float amount)
    {
        if(background && m_foreground)
        {
            auto back_size = background->GetSprite()->GetSize();
            auto fore_size = m_foreground->GetSprite()->GetSize();

            float from = back_size.y / 2 - fore_size.y / 2;
            float to = -from;
            float y = Mathf::Lerp(from, to, amount, false);

            auto transform = m_foreground->GetTransform();
            auto pos = transform->GetLocalPosition();
            pos.y = y;
            transform->SetLocalPosition(pos);
        }
    }

    void UIScrollBar::SetRatio(float ratio)
    {
        GetForeground();

        if(background && m_foreground)
        {
            auto fore = m_foreground->GetSprite();
            auto back_size = background->GetSprite()->GetSize();
            auto fore_size = fore->GetSize();

            ratio = Mathf::Clamp01(ratio);
            float h = ratio * back_size.y;
            auto size = Vector2(fore_size.x, h);
            fore->SetSize(size);
            auto collider = m_foreground->GetGameObject()->GetComponent<BoxCollider>();
            collider->SetSize(size);

            if(!scroll_view.expired())
            {
                SetAmount(scroll_view.lock()->GetAmount());
            }
        }
    }
}
#include "UISlider.h"
#include "Label.h"
#include "GTString.h"
#include "Mathf.h"
#include "GameObject.h"
#include "BoxCollider.h"
#include "UICanvas.h"

namespace Galaxy3D
{
    void UISliderThumb::OnDrag(const Vector3 &delta)
    {
        if(!slider.expired())
        {
            slider.lock()->OnDragSliderThumb(delta);
        }
    }

    UISlider::UISlider():
        type(UISliderValueType::Float),
        value_min(0),
        value_max(1),
        m_amount(-1)
    {
    }

    void UISlider::OnDragSliderThumb(const Vector3 &delta)
    {
        auto pos = UICanvas::GetLastRayHit().point;
        auto local_pos = GetTransform()->InverseTransformPoint(pos);
        float x = (local_pos - m_slider_collider->GetCenter()).x;
        float v = 0.5f + x / m_slider_collider->GetSize().x;

        SetAmount(v);
    }

    void UISlider::OnPress(bool press)
    {
        if(press)
        {
            auto pos = UICanvas::GetLastRayHit().point;
            auto local_pos = GetTransform()->InverseTransformPoint(pos);
            float x = (local_pos - m_slider_collider->GetCenter()).x;
            float v = 0.5f + x / m_slider_collider->GetSize().x;

            SetAmount(v);
        }
    }

    void UISlider::SetAmount(float v)
    {
        if(!m_slider_collider)
        {
            m_slider_collider = GetGameObject()->GetComponent<BoxCollider>();
        }
        if(thumb)
        {
            if(!m_thumb_collider)
            {
                m_thumb_collider = thumb->GetGameObject()->GetComponent<BoxCollider>();
            }
        }

        v = Mathf::Clamp01(v);
        if(!Mathf::FloatEqual(v, m_amount))
        {
            m_amount = v;

            if(label)
            {
                std::string text;
                if(type == UISliderValueType::Int)
                {
                    text = GTString::ToString(GetValue<int>()).str;
                }
                else
                {
                    text = GTString::ToString(GetValue<float>()).str;
                }
                label->SetText(text);
            }

            if(thumb)
            {
                float x = (m_amount - 0.5f) * m_slider_collider->GetSize().x;
                x += m_slider_collider->GetCenter().x;

                auto center = m_thumb_collider->GetCenter();
                thumb->GetTransform()->SetLocalPosition(Vector3(x - center.x, -center.y, 0));
            }

            OnValueChanged();
        }
    }
}
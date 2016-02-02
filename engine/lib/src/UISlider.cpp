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
        value(1),
        amount(1)
    {
    }

    void UISlider::Init()
    {
        m_slider_collider = GetGameObject()->GetComponent<BoxCollider>();
        if(thumb)
        {
            m_thumb_collider = thumb->GetGameObject()->GetComponent<BoxCollider>();
        }

        SetValue(amount, true);
    }

    void UISlider::OnDragSliderThumb(const Vector3 &delta)
    {
        auto pos = UICanvas::GetLastRayHit().point;
        auto local_pos = GetTransform()->InverseTransformPoint(pos);
        float x = (local_pos - m_slider_collider->GetCenter()).x;
        float v = 0.5f + x / m_slider_collider->GetSize().x;

        SetValue(v);
    }

    void UISlider::OnPress(bool press)
    {
        if(press)
        {
            auto pos = UICanvas::GetLastRayHit().point;
            auto local_pos = GetTransform()->InverseTransformPoint(pos);
            float x = (local_pos - m_slider_collider->GetCenter()).x;
            float v = 0.5f + x / m_slider_collider->GetSize().x;

            SetValue(v);
        }
    }

    void UISlider::SetValue(float v, bool force)
    {
        v = Mathf::Clamp01(v);
        if(force || !Mathf::FloatEqual(v, amount))
        {
            amount = v;
            value = value_min + amount * (value_max - value_min);

            if(label)
            {
                std::string text;
                if(type == UISliderValueType::Int)
                {
                    text = GTString::ToString((int) value).str;
                }
                else
                {
                    text = GTString::ToString(value).str;
                }
                label->SetText(text);
            }

            if(thumb)
            {
                float x = (amount - 0.5f) * m_slider_collider->GetSize().x;
                x += m_slider_collider->GetCenter().x;

                auto center = m_thumb_collider->GetCenter();
                thumb->GetTransform()->SetLocalPosition(Vector3(x - center.x, -center.y, 0));
            }

            OnValueChanged();
        }
    }
}
#ifndef __UISlider_h__
#define __UISlider_h__

#include "UIEventListener.h"
#include "Vector2.h"

namespace Galaxy3D
{
    class UISlider;
    class Label;
    class BoxCollider;

    class UISliderThumb : public UIEventListener
    {
    public:
        std::weak_ptr<UISlider> slider;

        virtual void OnDrag(const Vector3 &delta);
    };

    struct UISliderValueType
    {
        enum Enum
        {
            Int,
            Float,
        };
    };

    class UISlider : public UIEventListener
    {
    public:
        std::shared_ptr<UISliderThumb> thumb;
        std::shared_ptr<Label> label;
        UISliderValueType::Enum type;
        float value_min;
        float value_max;
        float value;
        float amount;//0.0~1.0

        UISlider();
        virtual void OnPress(bool press);
        void OnDragSliderThumb(const Vector3 &delta);

    protected:
        virtual void Start();
        virtual void OnValueChanged() {}

    private:
        std::shared_ptr<BoxCollider> m_slider_collider;
        std::shared_ptr<BoxCollider> m_thumb_collider;

        void SetValue(float v, bool force = false);
    };
}

#endif
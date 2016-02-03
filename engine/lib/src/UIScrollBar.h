#ifndef __UIScrollBar_h__
#define __UIScrollBar_h__

#include "UIEventListener.h"
#include "SpriteNode.h"

namespace Galaxy3D
{
    class UIScrollView;
    class UIScrollBar;

    class UIScrollBarThumb : public UIEventListener
    {
    public:
        std::weak_ptr<UIScrollBar> scroll_bar;

        virtual void OnDragStart();
        virtual void OnDrag(const Vector3 &delta);

    private:
        Vector3 m_down_pos;
    };

    class UIScrollBar : public UIEventListener
    {
    public:
        std::weak_ptr<UIScrollView> scroll_view;
        std::shared_ptr<SpriteNode> background;
        std::shared_ptr<UIScrollBarThumb> thumb;

        void SetAmount(float amount);
        void SetRatio(float ratio);
        void OnDragSliderThumbStart();
        void OnDragSliderThumb(const Vector3 &delta);

    private:
        std::shared_ptr<SpriteNode> m_foreground;
        float m_down_amount;

        std::shared_ptr<SpriteNode> GetForeground();
    };
}

#endif
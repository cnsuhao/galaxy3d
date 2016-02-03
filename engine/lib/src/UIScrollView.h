#ifndef __UIScrollView_h__
#define __UIScrollView_h__

#include "UIEventListener.h"
#include "Vector2.h"

namespace Galaxy3D
{
    class UIScrollBar;

    class UIScrollView : public UIEventListener
    {
    public:
        std::shared_ptr<GameObject> scroll_target;
        std::shared_ptr<UIScrollBar> scroll_bar;

        UIScrollView();
        void Init();
        void SetAmount(float amount);
        float GetAmount() const {return m_amount;}

    private:
        float m_amount;
        Vector2 m_target_size;
        Vector2 m_view_size;
        Vector2 m_view_pos;
    };
}

#endif
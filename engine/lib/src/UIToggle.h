#ifndef __UIToggle_h__
#define __UIToggle_h__

#include "UIEventListener.h"

namespace Galaxy3D
{
    class UIToggle : public UIEventListener
    {
    public:
        std::shared_ptr<GameObject> checkmark;

        UIToggle():
            m_value(-1)
        {}
        void SetValue(bool check);
        bool GetValue() const {return m_value == 1;}
        virtual void OnClick();
        virtual void OnValueChanged() {}

    private:
        char m_value;
    };
}

#endif
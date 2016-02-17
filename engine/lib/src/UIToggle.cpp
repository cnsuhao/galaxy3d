#include "UIToggle.h"
#include "GameObject.h"

namespace Galaxy3D
{
    void UIToggle::SetValue(bool check)
    {
        char value = (check ? 1 : 0);

        if(value != m_value)
        {
            m_value = check;

            if(checkmark)
            {
                checkmark->SetActive(check);
            }

            OnValueChanged(check);
        }
    }

    void UIToggle::OnClick()
    {
        if(checkmark)
        {
            SetValue(!(m_value == 1));
        }
    }
}
#include "UIToggle.h"
#include "GameObject.h"

namespace Galaxy3D
{
    void UIToggle::SetValue(bool check)
    {
        if((check ? 1 : 0) != m_value)
        {
            m_value = check;

            if(checkmark)
            {
                checkmark->SetActive(check);
            }

            OnValueChanged();
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
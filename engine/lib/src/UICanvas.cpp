#include "UICanvas.h"
#include "Screen.h"

namespace Galaxy3D
{
    void UICanvas::Start()
    {
        if(m_width == 0 || m_height == 0)
        {
            m_width = Screen::GetWidth();
            m_height = Screen::GetHeight();
        }
    }

    void UICanvas::SetSize(int w, int h)
    {
        m_width = w;
        m_height = h;
    }
}
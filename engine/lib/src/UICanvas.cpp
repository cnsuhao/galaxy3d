#include "UICanvas.h"
#include "Screen.h"
#include "Transform.h"

namespace Galaxy3D
{
    int UICanvas::GetWidth()
    {
        if(m_width == 0)
        {
            m_width = Screen::GetWidth();
        }

        return m_width;
    }

    int UICanvas::GetHeight()
    {
        if(m_height == 0)
        {
            m_height = Screen::GetHeight();
        }

        return m_height;
    }

    void UICanvas::SetSize(int w, int h)
    {
        m_width = w;
        m_height = h;
    }

    void UICanvas::AnchorTransform(std::shared_ptr<Transform> &t, const Vector4 &anchor)
    {
        int canvas_w = GetWidth();
        int canvas_h = GetHeight();

        float x = (anchor.x - 0.5f) * canvas_w + anchor.z;
        float y = (anchor.y - 0.5f) * canvas_h + anchor.w;
        Vector3 pos_local = Vector3(x, y, 0);

        t->SetPosition(GetTransform()->TransformPoint(pos_local));
    }
}
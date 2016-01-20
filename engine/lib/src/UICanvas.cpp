#include "UICanvas.h"
#include "Screen.h"
#include "Transform.h"
#include "Input.h"
#include "GameObject.h"
#include "GTTime.h"

namespace Galaxy3D
{
    class MouseOrTouch
    {
    public:
        KeyCode::Enum key;
        Vector2 pos;				// Current position of the mouse or touch event
        Vector2 last_pos;			// Previous position of the mouse or touch event
        Vector2 delta;		 	    // Delta since last update
        Vector2 total_delta;	    // Delta since the event started being tracked

        std::weak_ptr<GameObject> last;			// Last object under the touch or mouse
        std::weak_ptr<GameObject> current;		// Current game object under the touch or mouse
        std::weak_ptr<GameObject> pressed;		// Last game object to receive OnPress
        std::weak_ptr<GameObject> dragged;		// Game object that's being dragged

        float press_time;	// When the touch event started
        float click_time;	// The last time a click event was sent out

        bool touch_began;
        bool press_started;
        bool drag_started;
        int ignore_delta;

        MouseOrTouch():
            key(KeyCode::None),
            press_time(0),
            click_time(0),
            touch_began(true),
            press_started(false),
            drag_started(false),
            ignore_delta(0)
        {}

        /// <summary>
        /// Delta time since the touch operation started.
        /// </summary>
        float GetDeltaTime() {
            return GTTime::GetRealTimeSinceStartup() - press_time;
        }
    };

    static int g_current_mouse = -1;
    static MouseOrTouch g_mouse[3];
    static MouseOrTouch *g_current_touch = NULL;

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

    void UICanvas::Update()
    {
        ProcessMouse();
    }

    void UICanvas::ProcessMouse()
    {
        bool is_pressed = false;
        bool just_pressed = false;

        for(int i=0; i<3; i++)
        {
            if(Input::GetMouseButtonDown(i))
            {
                g_current_mouse = i;
                just_pressed = true;
                is_pressed = true;
            }
            else if (Input::GetMouseButton(i))
            {
                g_current_mouse = i;
                is_pressed = true;
            }
        }

        g_current_touch = &g_mouse[0];
    }
}
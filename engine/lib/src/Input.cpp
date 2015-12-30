#include "Input.h"
#include <list>

std::vector<Galaxy3D::Touch> g_input_touches;
std::list<Galaxy3D::Touch> g_input_touch_buffer;
bool g_key_down[Galaxy3D::KeyCode::COUNT];
bool g_key[Galaxy3D::KeyCode::COUNT];
bool g_key_up[Galaxy3D::KeyCode::COUNT];
bool g_key_held[Galaxy3D::KeyCode::COUNT];

namespace Galaxy3D
{
	bool Input::m_multi_touch_enabled = false;

	int Input::GetTouchCount()
	{
		return (int) g_input_touches.size();
	}

	const Touch *Input::GetTouch(int index)
	{
		if(index >= 0 && index < (int) g_input_touches.size())
        {
            return &g_input_touches[index];
        }
        
        return NULL;
	}

	void Input::Update()
	{
		g_input_touches.clear();
		if(!g_input_touch_buffer.empty())
		{
			g_input_touches.push_back(g_input_touch_buffer.front());
			g_input_touch_buffer.pop_front();
		}

        memset(g_key_down, 0, sizeof(g_key_down));
        memset(g_key_up, 0, sizeof(g_key_up));
	}

    bool Input::GetKeyDown(KeyCode::Enum key)
    {
        return g_key_down[key];
    }

    bool Input::GetKey(KeyCode::Enum key)
    {
        return g_key[key];
    }

    bool Input::GetKeyUp(KeyCode::Enum key)
    {
        return g_key_up[key];
    }
}
#include "Input.h"
#include <list>

std::vector<Galaxy3D::Touch> g_input_touches;
std::list<Galaxy3D::Touch> g_input_touch_buffer;

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
        
        return nullptr;
	}

	void Input::Update()
	{
		g_input_touches.clear();
		if(!g_input_touch_buffer.empty())
		{
			g_input_touches.push_back(g_input_touch_buffer.front());
			g_input_touch_buffer.pop_front();
		}
	}
}
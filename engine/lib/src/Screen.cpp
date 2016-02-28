#include "Screen.h"

namespace Galaxy3D
{
	int Screen::m_width;
	int Screen::m_height;
	ScreenOrientation::Enum Screen::m_orientation;
	std::vector<std::weak_ptr<IScreenResizeEventListener>> Screen::m_resize_listeners;

	void Screen::SetSize(int w, int h)
	{
		m_width = w;
		m_height = h;
	}

	void Screen::Resize(int w, int h)
	{
		m_width = w;
		m_height = h;

		for(auto &i : m_resize_listeners)
		{
			if(!i.expired())
			{
				i.lock()->OnScreenResize(m_width, m_height);
			}
		}
	}

	void Screen::AddResizeListener(const std::weak_ptr<IScreenResizeEventListener> &listener)
	{
		m_resize_listeners.push_back(listener);
	}
}
#include "Screen.h"

namespace Galaxy3D
{
	int Screen::m_width;
	int Screen::m_height;
	ScreenOrientation::Enum Screen::m_orientation;

	void Screen::SetSize(int w, int h)
	{
		m_width = w;
		m_height = h;
	}
}
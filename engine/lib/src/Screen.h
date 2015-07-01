#ifndef __Screen_h__
#define __Screen_h__

namespace Galaxy3D
{
	class Screen
	{
	public:
		static int GetWidth() {return m_width;}
		static int GetHeight() {return m_height;}
		static void SetSize(int w, int h);

	private:
		static int m_width;
		static int m_height;
	};
}

#endif
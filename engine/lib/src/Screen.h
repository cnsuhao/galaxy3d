#ifndef __Screen_h__
#define __Screen_h__

#include <vector>
#include <memory>

namespace Galaxy3D
{
	struct ScreenOrientation
	{
		enum Enum
		{
			Identity,
			Orientation90,
			Orientation180,
			Orientation270,
		};
	};

	class IScreenResizeEventListener
	{
	public:
		virtual void OnScreenResize(int width, int height) = 0;
	};

	class Screen
	{
	public:
		static int GetWidth() {return m_width;}
		static int GetHeight() {return m_height;}
		static void SetSize(int w, int h);
		static void Resize(int w, int h);
		static void SetOrientation(ScreenOrientation::Enum orientation) {m_orientation = orientation;}
		static ScreenOrientation::Enum GetOrientation() {return m_orientation;}
		static void AddResizeListener(const std::weak_ptr<IScreenResizeEventListener> &listener);
		
	private:
		static int m_width;
		static int m_height;
		static ScreenOrientation::Enum m_orientation;
		static std::vector<std::weak_ptr<IScreenResizeEventListener>> m_resize_listeners;
	};
}

#endif
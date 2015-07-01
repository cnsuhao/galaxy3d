#ifndef __Texture_h__
#define __Texture_h__

#include "Object.h"
#include "FilterMode.h"
#include "TextureWrapMode.h"

namespace Galaxy3D
{
	class Texture : public Object
	{
	public:
		virtual ~Texture() {}
		int GetWidth() const {return m_width;}
		int GetHeight() const {return m_height;}

	protected:
		FilterMode::Enum m_filter_mode;
		TextureWrapMode::Enum m_wrap_mode;
		int m_width;
		int m_height;

		Texture(int w, int h, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode):
			m_filter_mode(filter_mode),
			m_wrap_mode(wrap_mode),
			m_width(w),
			m_height(h)
		{}
	};
}

#endif
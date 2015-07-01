#include "Rect.h"
#include "Mathf.h"

namespace Galaxy3D
{
	Rect::Rect(float left, float top, float width, float height):
		left(left), top(top), width(width), height(height)
	{
	}

	bool Rect::operator ==(const Rect &r) const
	{
		return Mathf::FloatEqual(left, r.left) &&
			Mathf::FloatEqual(top, r.top) &&
			Mathf::FloatEqual(width, r.width) &&
			Mathf::FloatEqual(height, r.height);
	}

	bool Rect::operator !=(const Rect &r) const
	{
		return !(*this == r);
	}
}
#ifndef __Rect_h__
#define __Rect_h__

namespace Galaxy3D
{
	class Rect
	{
	public:
		Rect(float left=0, float top=0, float width=0, float height=0);
		bool operator ==(const Rect &r) const;
		bool operator !=(const Rect &r) const;
		void Set(float left, float top, float width, float height)
		{
			this->left = left;
			this->top = top;
			this->width = width;
			this->height = height;
		}

	public:
		float left;
        float top;
        float width;
        float height;
	};
}

#endif
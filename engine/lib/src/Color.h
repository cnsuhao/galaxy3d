#ifndef __Color_h__
#define __Color_h__

namespace Galaxy3D
{
	class Color
	{
	public:
		static Color Lerp(const Color &from, const Color &to, float t);
		
		Color(float r=0, float g=0, float b=0, float a=0);
		bool operator ==(const Color &c) const;
		bool operator !=(const Color &c) const;
		Color operator *(const Color &c) const;
		Color &operator *=(const Color &c);
		Color operator *(float v) const;

	public:
		float r;
        float g;
        float b;
        float a;
	};
}

#endif
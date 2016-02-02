#ifndef __Vector2_h__
#define __Vector2_h__

#include <string>

namespace Galaxy3D
{
	class Vector3;

	class Vector2
	{
	public:
		float x;
		float y;

		Vector2(float x=0, float y=0):x(x),y(y){}
		Vector2(const Vector3 &v3);
		Vector2 operator *(float value) const;
		Vector2 &operator *=(float value);
		Vector2 operator +(const Vector2 &value) const;
		Vector2 operator -(const Vector2 &value) const;
		bool operator ==(const Vector2 &value) const;
		bool operator !=(const Vector2 &value) const;
        std::string ToString() const;
		float Magnitude() const;
		float SqrMagnitude() const;
	};
}

#endif
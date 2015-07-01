#include "Ray.h"

namespace Galaxy3D
{
	Ray::Ray(const Vector3 &origin, const Vector3 &direction):
		origin(origin),
		mDirection(Vector3::Normalize(direction))
	{
	}

	Vector3 Ray::GetPoint(float distance) const
	{
		return origin + mDirection * distance;
	}
}
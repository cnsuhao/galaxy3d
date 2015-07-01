#include "Bounds.h"
#include <math.h>

namespace Galaxy3D
{
	Bounds::Bounds():
		center(0, 0, 0),
		size(0, 0, 0),
		extents(0, 0, 0),
		max_(0, 0, 0),
		min_(0, 0, 0)
	{
	}

	bool Bounds::Contains(const Vector3 &point) const
	{
		Vector3 dir = point - center;

		return !(fabs(dir.x) > extents.x || fabs(dir.y) > extents.y || fabs(dir.z) > extents.z);
	}

	void Bounds::Encapsulate(const Vector3 &point)
	{
		if(!Contains(point))
		{
			Vector3 dir = point - center;
			extents = Vector3::Max(extents, Vector3(fabs(dir.x), fabs(dir.y), fabs(dir.z)));
			size = extents * 2;
			max_ = center + extents;
			min_ = center - extents;
		}
	}
}
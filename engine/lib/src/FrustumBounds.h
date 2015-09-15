#ifndef __FrustumBounds_h__
#define __FrustumBounds_h__

#include "Vector4.h"
#include "Matrix4x4.h"

namespace Galaxy3D
{
	class FrustumBounds
	{
	public:
		//wvp check pos in model
		//vp check pos in world
		//p check pos in camera
		FrustumBounds(const Matrix4x4 &mat);
		bool ContainsPoint(const Vector3 &point) const;
		int ContainsSphere(const Vector3 &center, float radius) const;

	private:
		FrustumBounds(){}

	private:
		Vector4 m_frustum_planes[6];
	};
}

#endif
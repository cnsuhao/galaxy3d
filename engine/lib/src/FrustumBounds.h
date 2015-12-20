#ifndef __FrustumBounds_h__
#define __FrustumBounds_h__

#include "Vector4.h"
#include "Matrix4x4.h"

namespace Galaxy3D
{
    struct ContainsResult
    {
        enum Enum
        {
            In,
            Out,
            Cross
        };
    };

	class FrustumBounds
	{
	public:
		//wvp check pos in model
		//vp check pos in world
		//p check pos in camera
		FrustumBounds(const Matrix4x4 &mat);
		bool ContainsPoint(const Vector3 &point) const;
		int ContainsSphere(const Vector3 &center, float radius) const;
        int ContainsBounds(const Vector3 &center, const Vector3 &extents) const;
        float TestPlane(const Vector3 &point, int plane_index) const; 

	private:
        Vector4 m_frustum_planes[6];

		FrustumBounds(){}
	};
}

#endif
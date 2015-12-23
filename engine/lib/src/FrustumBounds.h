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
        //
        // 摘要:
        //     ///
        //     构建一个FrustumBounds, 投影类型为orthographic, in view space.
        //     ///
        static FrustumBounds FrustumBoundsOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
        //
        // 摘要:
        //     ///
        //     构建一个FrustumBounds, 投影类型为perspective.
        //     ///
        //
        // 参数:
        //   mat:
        //     wvp matrix, check pos in model space.
        //     vp matrix, check pos in world space.
        //     p matrix, check pos in view space.
		FrustumBounds(const Matrix4x4 &mat);
		bool ContainsPoint(const Vector3 &point) const;
		int ContainsSphere(const Vector3 &center, float radius) const;
        int ContainsBounds(const Vector3 &center, const Vector3 &extents) const;
        int ContainsPoints(const std::vector<Vector3> &points, const Matrix4x4 *matrix) const;
        float DistanceToPlane(const Vector3 &point, int plane_index) const; 

	private:
        Vector4 m_frustum_planes[6];

		FrustumBounds(){}
	};
}

#endif
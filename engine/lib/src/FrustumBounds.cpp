#include "FrustumBounds.h"

namespace Galaxy3D
{
	FrustumBounds::FrustumBounds(const Matrix4x4 &mat)
	{
		Vector4 left_plane;
		left_plane.x = mat.m30 + mat.m00;
		left_plane.y = mat.m31 + mat.m01;
		left_plane.z = mat.m32 + mat.m02;
		left_plane.w = mat.m33 + mat.m03;

		Vector4 right_plane;
		right_plane.x = mat.m30 - mat.m00;
		right_plane.y = mat.m31 - mat.m01;
		right_plane.z = mat.m32 - mat.m02;
		right_plane.w = mat.m33 - mat.m03;

		Vector4 bottom_plane;
		bottom_plane.x = mat.m30 + mat.m10;
		bottom_plane.y = mat.m31 + mat.m11;
		bottom_plane.z = mat.m32 + mat.m12;
		bottom_plane.w = mat.m33 + mat.m13;

		Vector4 top_plane;
		top_plane.x = mat.m30 - mat.m10;
		top_plane.y = mat.m31 - mat.m11;
		top_plane.z = mat.m32 - mat.m12;
		top_plane.w = mat.m33 - mat.m13;

		Vector4 near_plane;
		near_plane.x = mat.m30 + mat.m20;
		near_plane.y = mat.m31 + mat.m21;
		near_plane.z = mat.m32 + mat.m22;
		near_plane.w = mat.m33 + mat.m23;

		Vector4 far_plane;
		far_plane.x = mat.m30 - mat.m20;
		far_plane.y = mat.m31 - mat.m21;
		far_plane.z = mat.m32 - mat.m22;
		far_plane.w = mat.m33 - mat.m23;

		m_frustum_planes[0] = left_plane;
		m_frustum_planes[1] = right_plane;
		m_frustum_planes[2] = bottom_plane;
		m_frustum_planes[3] = top_plane;
		m_frustum_planes[4] = near_plane;
		m_frustum_planes[5] = far_plane;

		//normalize
		for(int i=0; i<6; i++)
		{
			Vector4 &plane = m_frustum_planes[i];

			float div = 1.0f / Vector3(plane.x, plane.y, plane.z).Magnitude();
			plane *= div;
		}
	}

	bool FrustumBounds::ContainsPoint(const Vector3 &point) const
	{
		for(int i=0; i<6; i++)
		{
			float distance = m_frustum_planes[i].x * point.x + m_frustum_planes[i].y * point.y + m_frustum_planes[i].z * point.z + m_frustum_planes[i].w;
			if(distance < 0)
			{
				return false;
			}
		}

		return true;
	}

	int FrustumBounds::ContainsSphere(const Vector3 &center, float radius) const
	{
		for(int i=0; i<6; i++)
		{
			float distance = m_frustum_planes[i].x * center.x + m_frustum_planes[i].y * center.y + m_frustum_planes[i].z * center.z + m_frustum_planes[i].w;
			if(distance < -radius)
			{
				//外部
				return -1;
			}

			if(fabs(distance) < radius)
			{
				//相交
				return 0;
			}
		}

		//内部
		return 1;
	}
}
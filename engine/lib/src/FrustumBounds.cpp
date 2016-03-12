#include "FrustumBounds.h"

namespace Galaxy3D
{
    FrustumBounds FrustumBounds::FrustumBoundsOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        float center_x = (left + right) * 0.5f;
        float center_y = (bottom + top) * 0.5f;
        float center_z = (zNear + zFar) * 0.5f;

        FrustumBounds bounds;

        Vector4 left_plane;
        left_plane.x = 1;
        left_plane.y = 0;
        left_plane.z = 0;
        left_plane.w = -Vector3(left, center_y, center_z).Dot(Vector3(1, 0, 0));

        Vector4 right_plane;
        right_plane.x = -1;
        right_plane.y = 0;
        right_plane.z = 0;
        right_plane.w = -Vector3(right, center_y, center_z).Dot(Vector3(-1, 0, 0));

        Vector4 bottom_plane;
        bottom_plane.x = 0;
        bottom_plane.y = 1;
        bottom_plane.z = 0;
        bottom_plane.w = -Vector3(center_x, bottom, center_z).Dot(Vector3(0, 1, 0));

        Vector4 top_plane;
        top_plane.x = 0;
        top_plane.y = -1;
        top_plane.z = 0;
        top_plane.w = -Vector3(center_x, top, center_z).Dot(Vector3(0, -1, 0));

        Vector4 near_plane;
        near_plane.x = 0;
        near_plane.y = 0;
        near_plane.z = 1;
        near_plane.w = -Vector3(center_x, center_y, zNear).Dot(Vector3(0, 0, 1));

        Vector4 far_plane;
        far_plane.x = 0;
        far_plane.y = 0;
        far_plane.z = -1;
        far_plane.w = -Vector3(center_x, center_y, zFar).Dot(Vector3(0, 0, -1));

        bounds.m_frustum_planes[0] = left_plane;
        bounds.m_frustum_planes[1] = right_plane;
        bounds.m_frustum_planes[2] = bottom_plane;
        bounds.m_frustum_planes[3] = top_plane;
        bounds.m_frustum_planes[4] = near_plane;
        bounds.m_frustum_planes[5] = far_plane;

        //normalize
        for(int i=0; i<6; i++)
        {
            Vector4 &plane = bounds.m_frustum_planes[i];

            float div = 1.0f / Vector3(plane.x, plane.y, plane.z).Magnitude();
            plane *= div;
        }

        return bounds;
    }

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

    float FrustumBounds::DistanceToPlane(const Vector3 &point, int plane_index) const
    {
        Vector4 p(point);
        p.w = 1;

        return Vector4::Dot(m_frustum_planes[plane_index], p);
    }

	bool FrustumBounds::ContainsPoint(const Vector3 &point) const
	{
		for(int i=0; i<6; i++)
		{
			float distance = DistanceToPlane(point, i);
			if(distance < 0)
			{
				return false;
			}
		}

		return true;
	}

	int FrustumBounds::ContainsSphere(const Vector3 &center, float radius) const
	{
        bool all_in = true;

		for(int i=0; i<6; i++)
		{
            float distance = DistanceToPlane(center, i);
			if(distance < -radius)
			{
				return ContainsResult::Out;
			}
            
            if(fabs(distance) < radius)
            {
                all_in = false;
            }
		}

        if(!all_in)
        {
            return ContainsResult::Cross;
        }

		return ContainsResult::In;
	}

    int FrustumBounds::ContainsPoints(const std::vector<Vector3> &points, const Matrix4x4 *matrix) const
    {
        std::vector<Vector3> ps(points.size());
        for(size_t i=0; i<points.size(); i++)
        {
            if(matrix != NULL)
            {
                ps[i] = matrix->MultiplyPoint3x4(points[i]);
            }
            else
            {
                ps[i] = points[i];
            }
        }

        int in_plane_count = 0;

        for(int i=0; i<6; i++)
        {
            int in_count = 0;

            for(size_t j=0; j<ps.size(); j++)
            {
                float distance = DistanceToPlane(ps[j], i);
                if(distance >= 0)
                {
                    in_count++;
                }
            }

            // all points in same side to one plane
            if(in_count == 0)
            {
                return ContainsResult::Out;
            }
            else if(in_count == (int) ps.size())
            {
                in_plane_count++;
            }
        }

        if(in_plane_count == 6)
        {
            return ContainsResult::In;
        }

        return ContainsResult::Cross;
    }

    int FrustumBounds::ContainsBounds(const Vector3 &center, const Vector3 &extents) const
    {
        std::vector<Vector3> corners(8);
        corners[0] = center + Vector3(extents.x, extents.y, extents.z);
        corners[1] = center + Vector3(-extents.x, extents.y, extents.z);
        corners[2] = center + Vector3(-extents.x, extents.y, -extents.z);
        corners[3] = center + Vector3(extents.x, extents.y, -extents.z);
        corners[4] = center + Vector3(extents.x, -extents.y, extents.z);
        corners[5] = center + Vector3(-extents.x, -extents.y, extents.z);
        corners[6] = center + Vector3(-extents.x, -extents.y, -extents.z);
        corners[7] = center + Vector3(extents.x, -extents.y, -extents.z);

        return ContainsPoints(corners, NULL);
    }
}
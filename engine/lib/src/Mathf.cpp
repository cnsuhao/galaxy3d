#include "Mathf.h"

namespace Galaxy3D
{
	const float Mathf::Epsilon = FLT_EPSILON;
	const float Mathf::PI = 3.1415926f;
	const float Mathf::Deg2Rad = 0.0174533f;
	const float Mathf::Rad2Deg = 57.2958f;
	const float Mathf::MaxFloatValue = FLT_MAX;
    const float Mathf::MinFloatValue = -FLT_MAX;

	float Mathf::Lerp(float from, float to, float t, bool clamp_01)
	{
		if(clamp_01)
		{
			t = Mathf::Clamp01(t);
		}
		
		return from + (to - from) * t; 
	}

	float Mathf::Round(float f)
	{
		float up = ceil(f);
		float down = floor(f);

		if(f - down < 0.5f)
		{
			return down;
		}
		else if(up - f < 0.5f)
		{
			return up;
		}
		else
		{
			return ((((int) up) % 2) == 0 ? up : down);
		}
	}

	int Mathf::RoundToInt(float f)
	{
		return (int) Round(f);
	}

	bool Mathf::RayPlaneIntersection(const Ray &ray, const Vector3 &plane_normal, const Vector3 &plane_point, float &t)
	{
		float n_dot_d = ray.GetDirection().Dot(plane_normal);

		if(FloatEqual(n_dot_d, 0))
		{
			return false;
		}

		t = (plane_point - ray.origin).Dot(plane_normal) / n_dot_d;
		return true;
	}

	bool Mathf::RayBoundsIntersection(const Ray &ray, const Bounds &box, float &t)
	{
		const Vector3 &dir = ray.GetDirection();

		float t_x_min;
		float t_x_max;
		float t_y_min;
		float t_y_max;
		float t_z_min;
		float t_z_max;

		if(FloatEqual(dir.x, 0))
		{
			t_x_min = MinFloatValue;
			t_x_max = MaxFloatValue;
		}
		else
		{
			float inv_x = 1 / dir.x;

			if(inv_x > 0)
			{
				t_x_min = (box.min_.x - ray.origin.x) * inv_x;
				t_x_max = (box.max_.x - ray.origin.x) * inv_x;
			}
			else
			{
				t_x_min = (box.max_.x - ray.origin.x) * inv_x;
				t_x_max = (box.min_.x - ray.origin.x) * inv_x;
			}
		}

		if(FloatEqual(dir.y, 0))
		{
			t_y_min = MinFloatValue;
			t_y_max = MaxFloatValue;
		}
		else
		{
			float inv_y = 1 / dir.y;

			if(inv_y > 0)
			{
				t_y_min = (box.min_.y - ray.origin.y) * inv_y;
				t_y_max = (box.max_.y - ray.origin.y) * inv_y;
			}
			else
			{
				t_y_min = (box.max_.y - ray.origin.y) * inv_y;
				t_y_max = (box.min_.y - ray.origin.y) * inv_y;
			}
		}

		if(FloatEqual(dir.z, 0))
		{
			t_z_min = MinFloatValue;
			t_z_max = MaxFloatValue;
		}
		else
		{
			float inv_z = 1 / dir.z;

			if(inv_z > 0)
			{
				t_z_min = (box.min_.z - ray.origin.z) * inv_z;
				t_z_max = (box.max_.z - ray.origin.z) * inv_z;
			}
			else
			{
				t_z_min = (box.max_.z - ray.origin.z) * inv_z;
				t_z_max = (box.min_.z - ray.origin.z) * inv_z;
			}
		}

		std::vector<float> mins;
		mins.push_back(t_x_min);
		mins.push_back(t_y_min);
		mins.push_back(t_z_min);

		float min = Max<float>(mins);

		std::vector<float> maxs;
		maxs.push_back(t_x_max);
		maxs.push_back(t_y_max);
		maxs.push_back(t_z_max);

		float max = Min<float>(maxs);

		if(min > max)
		{
			return false;
		}

		if(!box.Contains(ray.GetPoint(min)))
		{
			return false;
		}

		t = min;

		return true;
	}
}
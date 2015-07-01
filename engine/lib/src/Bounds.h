#ifndef __Bounds_h__
#define __Bounds_h__

#include "Vector3.h"

namespace Galaxy3D
{
	//AABB
	struct Bounds
    {
		Bounds();
        Bounds(const Vector3 &center, const Vector3 &size):
			center(center),
			size(size),
			extents(size * 0.5f),
			max_(center + extents),
			min_(center - extents)
		{}

        Vector3 center;
        Vector3 extents;
        Vector3 max_;
        Vector3 min_;
        Vector3 size;

		bool Contains(const Vector3 &point) const;
		void Encapsulate(const Vector3 &point);

		/*
        bool Contains(Vector3 point);
        void Encapsulate(Bounds bounds);
        void Encapsulate(Vector3 point);
        void Expand(float amount);
        void Expand(Vector3 amount);
        bool IntersectRay(Ray ray);
        bool IntersectRay(Ray ray, out float distance);
        bool Intersects(Bounds bounds);
        void SetMinMax(Vector3 min, Vector3 max);
        float SqrDistance(Vector3 point);
		*/
    };
}

#endif
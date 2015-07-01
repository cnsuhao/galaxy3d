#ifndef __Ray_h__
#define __Ray_h__

#include "Vector3.h"

namespace Galaxy3D
{
	struct Ray
    {
        Ray(const Vector3 &origin, const Vector3 &direction);
		const Vector3 &GetDirection() const { return mDirection; }
		void SetDirection(const Vector3 &value) { mDirection = Vector3::Normalize(value); }
        
        Vector3 origin;

        Vector3 GetPoint(float distance) const;

	private:
		Vector3 mDirection;
    };
}

#endif
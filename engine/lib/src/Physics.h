#ifndef __Physics_h__
#define __Physics_h__

#include "Vector3.h"

namespace Galaxy3D
{
    class Physics
    {
    public:
        static void Init();
        static void Step();
        static void Done();
        static void AddRigidBody(void *shape, void *body);
        static bool RayCast(const Vector3 &from, const Vector3 &dir, float length, Vector3 &hit, Vector3 &normal);
    };
}

#endif
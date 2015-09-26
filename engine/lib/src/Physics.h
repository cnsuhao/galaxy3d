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
        static void CreateTerrainRigidBody(
            int size_heightmap,
            short *data,
            float unit_size,
            float unit_height);
        static bool RarCast(const Vector3 &from, const Vector3 &dir, float length, Vector3 &hit, Vector3 &normal);
    };
}

#endif
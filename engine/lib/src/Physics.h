#ifndef __Physics_h__
#define __Physics_h__

#include "Vector3.h"
#include "Terrain.h"

namespace Galaxy3D
{
    class Physics
    {
    public:
        static void Init();
        static void Step();
        static void Done();
        static void CreateTerrainRigidBody(const Terrain *terrain);
        static bool RarCast(const Vector3 &from, const Vector3 &dir, float length, Vector3 &hit, Vector3 &normal);
    };
}

#endif
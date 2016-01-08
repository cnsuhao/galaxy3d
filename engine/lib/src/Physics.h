#ifndef __Physics_h__
#define __Physics_h__

#include "Vector3.h"
#include <memory>

namespace Galaxy3D
{
    class Terrain;

    class Physics
    {
    public:
        static void Init();
        static void Step();
        static void Done();
        static void CreateTerrainRigidBody(const std::shared_ptr<Terrain> &terrain);
        static void AddRigidBody(void *shape, void *body);
        static bool RayCast(const Vector3 &from, const Vector3 &dir, float length, Vector3 &hit, Vector3 &normal);
    };
}

#endif
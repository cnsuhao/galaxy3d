#ifndef __Physics_h__
#define __Physics_h__

#include "Vector3.h"
#include "Collider.h"

namespace Galaxy3D
{
    struct RaycastHit
    {
        Vector3 point;
        Vector3 normal;
        std::shared_ptr<Collider> collider;
    };

    class Physics
    {
    public:
        static void Init();
        static void Step();
        static void Done();
        static void AddRigidBody(void *shape, void *body);
        static bool RayCast(const Vector3 &from, const Vector3 &dir, float length, RaycastHit &hit);
    };
}

#endif
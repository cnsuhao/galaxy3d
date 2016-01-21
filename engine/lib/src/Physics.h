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
        // bullet use short for mask, and 0 ~ 5 is used, so gameobject layer must be in range 0 ~ 9 (6 ~ 15 - 6)
        static void AddRigidBody(void *shape, void *body);
        static bool Raycast(const Vector3 &from, const Vector3 &dir, float length, RaycastHit &hit);
        static std::vector<RaycastHit> RaycastAll(const Vector3 &from, const Vector3 &dir, float length, int layer_mask = -1);
    };
}

#endif
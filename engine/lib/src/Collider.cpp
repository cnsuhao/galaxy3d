#include "Collider.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Galaxy3D
{
    void Collider::OnEnable()
    {
        btRigidBody *body = (btRigidBody *) m_rigidbody;
        Physics::RestoreRigidBody(body);
    }

    void Collider::OnDisable()
    {
        btRigidBody *body = (btRigidBody *) m_rigidbody;
        Physics::RemoveRigidBody(body);
    }
}
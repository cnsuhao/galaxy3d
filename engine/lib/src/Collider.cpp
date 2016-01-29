#include "Collider.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Galaxy3D
{
    Collider::~Collider()
    {
        OnDisable();
    }

    void Collider::OnEnable()
    {
        if(!m_in_world)
        {
            m_in_world = true;

            btRigidBody *body = (btRigidBody *) m_rigidbody;
            Physics::RestoreRigidBody(body);
        }
    }

    void Collider::OnDisable()
    {
        if(m_in_world)
        {
            m_in_world = false;

            btRigidBody *body = (btRigidBody *) m_rigidbody;
            Physics::RemoveRigidBody(body);
        }
    }
}
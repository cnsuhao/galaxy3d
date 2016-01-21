#include "BoxCollider.h"
#include "Transform.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace Galaxy3D
{
    void BoxCollider::Start()
    {
        auto pos = GetTransform()->GetPosition();
        auto rot = GetTransform()->GetRotation();
        auto sca = GetTransform()->GetScale();

        btBoxShape *shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
        shape->setLocalScaling(btVector3(m_size.x * sca.x, m_size.y * sca.y, m_size.z * sca.z));

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(pos.x + m_center.x, pos.y + m_center.y, pos.z + m_center.z));
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

        btScalar mass(0);
        btVector3 local_inertia(0, 0, 0);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState *motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, shape, local_inertia);
        btRigidBody *body = new btRigidBody(rbInfo);
        body->setRollingFriction(1);
        body->setFriction(1);
        body->setUserPointer(this);

        Physics::AddRigidBody(shape, body);

        m_rigidbody = body;
    }

    void BoxCollider::OnTranformChanged()
    {
        if(m_rigidbody != NULL)
        {
            auto pos = GetTransform()->GetPosition();
            auto rot = GetTransform()->GetRotation();
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(pos.x + m_center.x, pos.y + m_center.y, pos.z + m_center.z));
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

            btRigidBody *body = (btRigidBody *) m_rigidbody;
            body->setWorldTransform(transform);
        }
    }
}
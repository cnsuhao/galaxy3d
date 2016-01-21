#include "Physics.h"
#include "Debug.h"
#include "GTTime.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace Galaxy3D
{
    static btDefaultCollisionConfiguration *g_config = NULL;
    static btCollisionDispatcher *g_dispatcher = NULL;
    static btDbvtBroadphase *g_broadphase = NULL;
    static btSequentialImpulseConstraintSolver *g_solver = NULL;
    static btDiscreteDynamicsWorld *g_dynamics_world = NULL;
    static btAlignedObjectArray<btCollisionShape *> g_collision_shapes;

    void Physics::Init()
    {
        if(g_config != NULL)
        {
            Debug::Log("Physics had init error");
        }
        
        g_config = new btDefaultCollisionConfiguration();
        g_dispatcher = new btCollisionDispatcher(g_config);
        g_broadphase = new btDbvtBroadphase();
        g_solver = new btSequentialImpulseConstraintSolver();
        g_dynamics_world = new btDiscreteDynamicsWorld(g_dispatcher, g_broadphase, g_solver, g_config);
        g_dynamics_world->setGravity(btVector3(0, -10, 0));
    }

    void Physics::AddRigidBody(void *shape, void *body, int layer)
    {
        g_collision_shapes.push_back((btCollisionShape *) shape);
        g_dynamics_world->addRigidBody((btRigidBody *) body, 1 << layer, -1);
    }

    bool Physics::Raycast(const Vector3 &from, const Vector3 &dir, float length, RaycastHit &hit, int layer_mask)
    {
        Vector3 to = from + Vector3::Normalize(dir) * length;
        btVector3 from_(from.x, from.y, from.z);
        btVector3 to_(to.x, to.y, to.z);

        btCollisionWorld::ClosestRayResultCallback closest(from_, to_);
        closest.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
        closest.m_collisionFilterMask = layer_mask;

        g_dynamics_world->rayTest(from_, to_, closest);

        if(closest.hasHit())
        {
            btVector3 pos = from_.lerp(to_, closest.m_closestHitFraction);
            btVector3 nor = closest.m_hitNormalWorld;

            hit.point = Vector3(pos.x(), pos.y(), pos.z());
            hit.normal = Vector3(nor.x(), nor.y(), nor.z());

            if(closest.m_collisionObject != NULL)
            {
                void *user_data = closest.m_collisionObject->getUserPointer();
                if(user_data != NULL)
                {
                    Collider *collider = (Collider *) user_data;
                    hit.collider = std::dynamic_pointer_cast<Collider>(collider->GetComponentPtr());
                }
            }

            return true;
        }

        return false;
    }

    std::vector<RaycastHit> Physics::RaycastAll(const Vector3 &from, const Vector3 &dir, float length, int layer_mask)
    {
        std::vector<RaycastHit> hits;

        Vector3 to = from + Vector3::Normalize(dir) * length;
        btVector3 from_(from.x, from.y, from.z);
        btVector3 to_(to.x, to.y, to.z);

        btCollisionWorld::AllHitsRayResultCallback all(from_, to_);
        all.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
        all.m_collisionFilterMask = layer_mask;

        g_dynamics_world->rayTest(from_, to_, all);

        if(all.hasHit())
        {
            for(int i=0; i<all.m_hitFractions.size(); i++)
            {
                RaycastHit hit;

                btVector3 pos = from_.lerp(to_, all.m_hitFractions[i]);
                btVector3 nor = all.m_hitNormalWorld[i];

                hit.point = Vector3(pos.x(), pos.y(), pos.z());
                hit.normal = Vector3(nor.x(), nor.y(), nor.z());

                if(all.m_collisionObjects[i] != NULL)
                {
                    void *user_data = all.m_collisionObjects[i]->getUserPointer();
                    if(user_data != NULL)
                    {
                        Collider *collider = (Collider *) user_data;
                        hit.collider = std::dynamic_pointer_cast<Collider>(collider->GetComponentPtr());
                    }
                }

                hits.push_back(hit);
            }
        }

        return hits;
    }

    void Physics::Step()
    {
        g_dynamics_world->stepSimulation(GTTime::GetDeltaTime());
    }

    void Physics::Done()
    {
        for(int i=g_dynamics_world->getNumCollisionObjects()-1; i>=0; i--)
        {
            btCollisionObject *obj = g_dynamics_world->getCollisionObjectArray()[i];
            btRigidBody *body = btRigidBody::upcast(obj);
            if(body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            g_dynamics_world->removeCollisionObject(obj);
            delete obj;
        }

        for(int i=0; i<g_collision_shapes.size(); i++)
        {
            btCollisionShape *shape = g_collision_shapes[i];
            delete shape;
        }
        g_collision_shapes.clear();

        delete g_dynamics_world;
        delete g_solver;
        delete g_broadphase;
        delete g_dispatcher;
        delete g_config;
    }
}
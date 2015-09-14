#include "Physics.h"
#include "Debug.h"
#include "GTTime.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace Galaxy3D
{
    btDefaultCollisionConfiguration *g_config = nullptr;
    btCollisionDispatcher *g_dispatcher = nullptr;
    btDbvtBroadphase *g_broadphase = nullptr;
    btSequentialImpulseConstraintSolver *g_solver = nullptr;
    btDiscreteDynamicsWorld *g_dynamics_world = nullptr;
    btAlignedObjectArray<btCollisionShape *> g_collision_shapes;
    short *g_terrain_data = nullptr;

    void Physics::Init()
    {
        if(g_config != nullptr)
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

    void Physics::CreateTerrainRigidBody(int size_heightmap, short *data, float height_min, float height_max)
    {
        if(g_terrain_data != nullptr)
        {
            delete [] g_terrain_data;
        }
        int point_count = size_heightmap * size_heightmap;
        g_terrain_data = new short[point_count];
        memcpy(g_terrain_data, data, point_count * sizeof(short));

        btHeightfieldTerrainShape *terrain = new btHeightfieldTerrainShape(
            size_heightmap,
            size_heightmap,
            g_terrain_data,
            1,
            height_min,
            height_max,
            1,
            PHY_SHORT,
            false);
        g_collision_shapes.push_back(terrain);

        btTransform ground_transform;
        ground_transform.setIdentity();
        ground_transform.setOrigin(btVector3((size_heightmap - 1) * 0.5f, (height_min + height_max) * 0.5f, (size_heightmap - 1) * 0.5f));

        btScalar mass(0);
        btVector3 local_inertia(0, 0, 0);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, terrain, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setRollingFriction(1);
        body->setFriction(1);

        g_dynamics_world->addRigidBody(body);
    }

    bool Physics::RarCast(const Vector3 &from, const Vector3 &dir, float length, Vector3 &hit, Vector3 &normal)
    {
        Vector3 to = from + Vector3::Normalize(dir) * length;
        btVector3 from_(from.x, from.y, from.z);
        btVector3 to_(to.x, to.y, to.z);

        btCollisionWorld::ClosestRayResultCallback closest(from_, to_);
        closest.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

        g_dynamics_world->rayTest(from_, to_, closest);

        if(closest.hasHit())
        {
            btVector3 pos = from_.lerp(to_, closest.m_closestHitFraction);
            btVector3 nor = closest.m_hitNormalWorld;

            hit = Vector3(pos.x(), pos.y(), pos.z());
            normal = Vector3(nor.x(), nor.y(), nor.z());

            return true;
        }

        return false;
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
        g_dynamics_world = nullptr;

        delete g_solver;
        g_solver = nullptr;

        delete g_broadphase;
        g_broadphase = nullptr;

        delete g_dispatcher;
        g_dispatcher = nullptr;

        delete g_config;
        g_config = nullptr;

        if(g_terrain_data != nullptr)
        {
            delete [] g_terrain_data;
        }
    }
}
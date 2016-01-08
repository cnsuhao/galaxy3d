#include "MeshCollider.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace Galaxy3D
{
    MeshCollider::~MeshCollider()
    {
        if(m_collider_data != NULL)
        {
            delete m_collider_data;
            m_collider_data = 0;
        }

        if(m_indices != NULL)
        {
            delete m_indices;
            m_indices = 0;
        }
    }

    void MeshCollider::Start()
    {
        auto &vertices = m_mesh->GetVertices();
        auto &indices = m_mesh->GetIndices();

        int index_count = 0;
        for(size_t i=0; i<indices.size(); i++)
        {
            index_count += indices[i].size();
        }
        int idnex_size = index_count * sizeof(unsigned short);
        m_indices = new unsigned short[idnex_size];

        int old_size = 0;
        for(size_t i=0; i<indices.size(); i++)
        {
            int size = indices[i].size();
            memcpy(&m_indices[old_size], &indices[i][0], size * sizeof(unsigned short));
            old_size += size;
        }

        btIndexedMesh mesh;
        mesh.m_numTriangles = index_count / 3;
        mesh.m_triangleIndexBase = (const unsigned char *) m_indices;
        mesh.m_triangleIndexStride = sizeof(unsigned short) * 3;
        mesh.m_numVertices = vertices.size();
        mesh.m_vertexBase = (const unsigned char *) &vertices[0];
        mesh.m_vertexStride = sizeof(VertexMesh);

        m_collider_data = new btTriangleIndexVertexArray();
        m_collider_data->addIndexedMesh(mesh, PHY_SHORT);

        btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(m_collider_data, true);

        btTransform ground_transform;
        ground_transform.setIdentity();
        auto pos = GetTransform()->GetPosition();
        ground_transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

        btScalar mass(0);
        btVector3 local_inertia(0, 0, 0);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setRollingFriction(1);
        body->setFriction(1);

        Physics::AddRigidBody(shape, body);
    }
}
#ifndef __MeshCollider_h__
#define __MeshCollider_h__

#include "Component.h"
#include "Mesh.h"

class btTriangleIndexVertexArray;

namespace Galaxy3D
{
    class MeshCollider : public Component
    {
    public:
        MeshCollider():
            m_collider_data(NULL),
            m_indices(NULL)
        {}
        virtual ~MeshCollider();
        virtual void Start();
        void SetMesh(const std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}

    private:
        std::shared_ptr<Mesh> m_mesh;
        btTriangleIndexVertexArray *m_collider_data;
        unsigned short *m_indices;
    };
}

#endif
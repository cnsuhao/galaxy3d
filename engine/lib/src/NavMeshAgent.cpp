#include "NavMeshAgent.h"
#include "NavMesh.h"
#include "Transform.h"

namespace Galaxy3D
{
    void NavMeshAgent::Start()
    {
        Vector3 pos = GetTransform()->GetPosition();
        int tri_index = NavMesh::FindTriangle(pos);
        if(tri_index >= 0)
        {
            pos = NavMesh::GetPosition(tri_index, 0, 0);
            GetTransform()->SetPosition(pos);
            m_navmesh_triangle_index = tri_index;
        }
    }

    void NavMeshAgent::Move(const Vector3 &offset)
    {
        if(m_navmesh_triangle_index < 0)
        {
            return;
        }

        Vector3 target = GetTransform()->GetPosition() + offset;

        NavMeshPath path;
        bool result = NavMesh::CalculatePath(GetTransform()->GetPosition(), m_navmesh_triangle_index, target, path);
    }
}
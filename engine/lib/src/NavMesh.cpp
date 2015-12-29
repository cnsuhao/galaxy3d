#include "NavMesh.h"
#include "GTFile.h"

namespace Galaxy3D
{
    std::vector<NavTriangle> NavMesh::m_triangles;
    std::vector<Vector3> NavMesh::m_vertices;
    std::vector<int> NavMesh::m_indices;

    void NavMesh::LoadFromFile(const std::string &file)
    {
        int file_size;
        char *bytes = (char *) GTFile::ReadAllBytes(file, &file_size);
        if(bytes != NULL)
        {
            char *p = bytes;

            int vertex_count;
            BUFFER_READ(vertex_count, p, 4);
            m_vertices.resize(vertex_count);
            BUFFER_READ(m_vertices[0], p, sizeof(Vector3) * vertex_count);

            int index_count;
            BUFFER_READ(index_count, p, 4);
            m_indices.resize(index_count);
            BUFFER_READ(m_indices[0], p, sizeof(int) * index_count);

            int triangle_count;
            BUFFER_READ(triangle_count, p, 4);
            m_triangles.resize(triangle_count);
            BUFFER_READ(m_triangles[0], p, sizeof(NavTriangle) * triangle_count);

            free(bytes);
        }
    }

    int NavMesh::FindTriangle(const Vector3 &pos)
    {
        int index = -1;

        Vector3 v = pos;
        v.y = 0;

        for(size_t i=0; i<m_triangles.size(); i++)
        {
            Vector3 v0 = m_vertices[m_triangles[i].edges[0].vertex_left];
            Vector3 v1 = m_vertices[m_triangles[i].edges[1].vertex_left];
            Vector3 v2 = m_vertices[m_triangles[i].edges[2].vertex_left];

            v0.y = 0;
            v1.y = 0;
            v2.y = 0;

            Vector3 c0 = (v1 - v0) * (v - v0);
            Vector3 c1 = (v2 - v1) * (v - v1);
            Vector3 c2 = (v0 - v2) * (v - v2);
            if( !(c0.y * c1.y < 0 ||
                c1.y * c2.y < 0 ||
                c2.y * c0.y < 0))
            {
                index = i;
                break;
            }
        }

        return index;
    }

    Vector3 NavMesh::GetPosition(int index, float x, float z)
    {
        Vector3 v(x, 0, z);

        auto &t = m_triangles[index];
        
        Vector3 v0 = m_vertices[t.edges[0].vertex_left];
        Vector3 v1 = m_vertices[t.edges[1].vertex_left];
        Vector3 v2 = m_vertices[t.edges[2].vertex_left];

        float u = ((v1.x - v2.x) * (v2.z - v.z) - (v2.x - v.x) * (v1.z - v2.z)) /
            ((v0.x - v2.x) * (v1.z - v2.z) - (v1.x - v2.x) * (v0.z - v2.z));
        float w = ((v.x - v2.x) - (v0.x - v2.x) * u) / (v1.x - v2.x);

        v.y = v0.y * u + v1.y * w + (1 - u - w) * v2.y;

        return v;
    }
}
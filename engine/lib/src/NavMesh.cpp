#include "NavMesh.h"
#include "GTFile.h"
#include "Mathf.h"
#include <list>

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

    bool NavMesh::IsInTriangle(const Vector3 &pos, int index)
    {
        Vector3 v = pos;
        v.y = 0;

        Vector3 v0 = m_vertices[m_triangles[index].edges[0].vertex_left];
        Vector3 v1 = m_vertices[m_triangles[index].edges[1].vertex_left];
        Vector3 v2 = m_vertices[m_triangles[index].edges[2].vertex_left];

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
            return true;
        }

        return false;
    }

    int NavMesh::FindTriangle(const Vector3 &pos)
    {
        int index = -1;

        for(size_t i=0; i<m_triangles.size(); i++)
        {
            if(IsInTriangle(pos, i))
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

        float div_u = (v0.x - v2.x) * (v1.z - v2.z) - (v1.x - v2.x) * (v0.z - v2.z);
        float div_w = v1.x - v2.x;

        if(Mathf::FloatEqual(div_u, 0) || Mathf::FloatEqual(div_w, 0))
        {
            v0 = m_vertices[t.edges[1].vertex_left];
            v1 = m_vertices[t.edges[0].vertex_left];
        }

        float u = ((v1.x - v2.x) * (v2.z - v.z) - (v2.x - v.x) * (v1.z - v2.z)) /
            ((v0.x - v2.x) * (v1.z - v2.z) - (v1.x - v2.x) * (v0.z - v2.z));
        float w = ((v.x - v2.x) - (v0.x - v2.x) * u) / (v1.x - v2.x);

        v.y = v0.y * u + v1.y * w + (1 - u - w) * v2.y;

        return v;
    }

    struct AStarNode
    {
        int triangle_index;
        AStarNode *parent;
        float g;
        float h;

        AStarNode():
            triangle_index(-1),
            parent(NULL),
            g(0),
            h(0)
        {
        }

        static bool Less(const AStarNode *a, const AStarNode *b)
        {
            return a->g + a->h < b->g + b->h;
        }

        static AStarNode *FindInList(std::list<AStarNode *> &list, int triangle_index)
        {
            for(auto &i : list)
            {
                if(i->triangle_index == triangle_index)
                {
                    return i;
                }
            }

            return NULL;
        }
    };

    std::list<AStarNode *> open;
    std::list<AStarNode *> close;

    bool NavMesh::CalculatePath(const Vector3 &source, int source_triangle_index, const Vector3 &target, NavMeshPath &path)
    {
        Vector3 begin = GetPosition(source_triangle_index, source.x, source.z);

        bool in_same_node = IsInTriangle(target, source_triangle_index);
        if(in_same_node)
        {
            Vector3 end = GetPosition(source_triangle_index, target.x, target.z);

            path.corners.clear();
            path.corners.push_back(begin);
            path.corners.push_back(end);
            return true;
        }

        AStarNode *first = new AStarNode();
        first->triangle_index = source_triangle_index;
        open.push_back(first);

        AStarNode *found_node = NULL;

        while(!open.empty())
        {
            open.sort(AStarNode::Less);
            auto node = open.front();
            open.remove(node);
            close.push_back(node);

            if(IsInTriangle(target, node->triangle_index))
            {
                found_node = node;
                break;
            }

            for(int i=0; i<3; i++)
            {
                int neighbor = m_triangles[node->triangle_index].edges[i].neighbor;

                if(neighbor >= 0)
                {
                    if(AStarNode::FindInList(close, neighbor) == NULL)
                    {
                        AStarNode *neighbor_node = new AStarNode();
                        neighbor_node->triangle_index = neighbor;
                        neighbor_node->parent = node;

                        auto &neighbor_triangle = m_triangles[neighbor_node->triangle_index];
                        Vector3 v0 = m_vertices[neighbor_triangle.edges[0].vertex_left];
                        Vector3 v1 = m_vertices[neighbor_triangle.edges[1].vertex_left];
                        Vector3 v2 = m_vertices[neighbor_triangle.edges[2].vertex_left];
                        Vector3 center = (v0 + v1 + v2) * (1.0f / 3);
                        center.y = 0;
                        Vector3 end = target;
                        end.y = 0;
                        neighbor_node->h = (center - end).SqrMagnitude();

                        {
                            auto &parent_triangle = m_triangles[neighbor_node->parent->triangle_index];
                            Vector3 parent_v0 = m_vertices[parent_triangle.edges[0].vertex_left];
                            Vector3 parent_v1 = m_vertices[parent_triangle.edges[1].vertex_left];
                            Vector3 parent_v2 = m_vertices[parent_triangle.edges[2].vertex_left];
                            Vector3 parent_center = (parent_v0 + parent_v1 + parent_v2) * (1.0f / 3);
                            center = (v0 + v1 + v2) * (1.0f / 3);

                            auto &shared_edge = m_triangles[node->triangle_index].edges[i];
                            Vector3 edge_left = m_vertices[shared_edge.vertex_left];
                            Vector3 edge_right = m_vertices[shared_edge.vertex_right];
                            Vector3 edge_center = (edge_left + edge_right) * 0.5f;

                            neighbor_node->g = neighbor_node->parent->g + 
                                (parent_center - edge_center).SqrMagnitude() +
                                (center - edge_center).SqrMagnitude();
                        }

                        AStarNode *node_in_open = AStarNode::FindInList(open, neighbor);

                        if(node_in_open == NULL)
                        {
                            open.push_back(neighbor_node);
                        }
                        else
                        {
                            if(neighbor_node->g < node_in_open->g)
                            {
                                node_in_open->g = neighbor_node->g;
                                node_in_open->parent = node;
                            }

                            delete neighbor_node;
                        }
                    }
                }
            }
        }

        if(found_node != NULL)
        {
            //path.corners.clear();
            //path.corners.push_back(begin);
            //path.corners.push_back(end);

            int depth = 0;
            auto node = found_node;
            while(node->parent != NULL)
            {
                node = node->parent;
                depth++;
            }

            return true;
        }

        return false;
    }
}
#ifndef __Mesh_h__
#define __Mesh_h__

#include "Object.h"
#include "GameObject.h"
#include "VertexType.h"
#include "Renderer.h"
#include <vector>

namespace Galaxy3D
{
    class Mesh : public Object
    {
    public:
        static std::shared_ptr<Mesh> Create();
        static std::shared_ptr<GameObject> LoadStaticMesh(const std::string &file);
        static std::shared_ptr<GameObject> LoadSkinnedMesh(const std::string &file);
        void SetVertices(const std::vector<VertexMesh> &vertices) {m_vertices = vertices;}
        void SetIndices(const std::vector<std::vector<unsigned short>> indices) {m_sub_indices = indices;}
        ID3D11Buffer *GetVertexBuffer();
        ID3D11Buffer *GetIndexBuffer();
        int GetIndexCount(int submesh) {return m_sub_indices[submesh].size();}

    private:
        std::vector<VertexMesh> m_vertices;
        std::vector<VertexSkinned> m_vertices_skinned;
        std::vector<std::vector<unsigned short>> m_sub_indices;
        std::vector<Matrix4x4> m_bind_poses;
        ID3D11Buffer *m_vertex_buffer;
        ID3D11Buffer *m_index_buffer;

        static std::shared_ptr<Mesh> ReadMesh(char *&p, Renderer *renderer, const std::string &dir, bool skin);
        Mesh();
    };
}

#endif
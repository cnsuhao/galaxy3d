#ifndef __Mesh_h__
#define __Mesh_h__

#include "Object.h"
#include "GameObject.h"
#include "VertexType.h"
#include "Renderer.h"
#include "Bounds.h"
#include <vector>

namespace Galaxy3D
{
    class Mesh : public Object
    {
    public:
        static std::shared_ptr<Mesh> Create();
        static std::shared_ptr<GameObject> LoadStaticMesh(const std::string &file);
        static std::shared_ptr<GameObject> LoadSkinnedMesh(const std::string &file);
        static std::shared_ptr<Mesh> CreateMeshCube();
        static std::shared_ptr<Mesh> CreateMeshSphere();
        static std::shared_ptr<Mesh> CreateMeshCone();
        ~Mesh();
        void SetVertices(const std::vector<VertexMesh> &vertices) {m_vertices = vertices;}
        std::vector<VertexMesh> &GetVertices() {return m_vertices;}
        std::vector<VertexSkinned> &GetVerticesSkinned() {return m_vertices_skinned;}
        void SetIndices(const std::vector<std::vector<unsigned short>> indices) {m_sub_indices = indices;}
        std::vector<std::vector<unsigned short>> &GetIndices() {return m_sub_indices;}
        BufferObject GetVertexBuffer();
        BufferObject GetIndexBuffer();
        int GetIndexCount(int submesh) {return m_sub_indices[submesh].size();}
        const std::vector<Matrix4x4> &GetBindPoses() const {return m_bind_poses;}
        void SetGuid(const std::string &guid) {m_guid = guid;}
        void CalculateBounds();

    private:
        std::string m_guid;
        std::vector<VertexMesh> m_vertices;
        std::vector<VertexSkinned> m_vertices_skinned;
        std::vector<std::vector<unsigned short>> m_sub_indices;
        std::vector<Matrix4x4> m_bind_poses;
        Bounds m_bounds;// aabb in local space
        BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;

        static std::shared_ptr<Mesh> ReadMesh(char *&p, Renderer *renderer, const std::string &dir, bool skin);
        static std::shared_ptr<Material> ReadMaterial(char *&p, const std::string &dir, bool skin);
        Mesh();
    };
}

#endif
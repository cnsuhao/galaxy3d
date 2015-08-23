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
        static std::shared_ptr<GameObject> LoadFromFile(const std::string &file);
        void SetVertices(const std::vector<VertexMesh> &vertices) {m_vertices = vertices;}

    private:
        std::vector<VertexMesh> m_vertices;
        std::vector<std::vector<unsigned short>> m_sub_indices;

        static void ReadMesh(char *&p, Renderer *renderer);
        Mesh();
    };
}

#endif
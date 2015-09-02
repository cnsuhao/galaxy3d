#ifndef __MeshRenderer_h__
#define __MeshRenderer_h__

#include "Renderer.h"
#include "Mesh.h"

namespace Galaxy3D
{
    class MeshRenderer : public Renderer
    {
    public:
        virtual void Render();
        void SetMesh(const std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}

    private:
        std::shared_ptr<Mesh> m_mesh;
    };
}

#endif
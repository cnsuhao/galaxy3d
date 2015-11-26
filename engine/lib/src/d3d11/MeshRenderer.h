#ifndef __MeshRenderer_h__
#define __MeshRenderer_h__

#include "Renderer.h"
#include "Mesh.h"

namespace Galaxy3D
{
    class MeshRenderer : public Renderer
    {
    public:
        void SetMesh(const std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}

    protected:
        virtual void Render(int material_index);

    private:
        std::shared_ptr<Mesh> m_mesh;
    };
}

#endif
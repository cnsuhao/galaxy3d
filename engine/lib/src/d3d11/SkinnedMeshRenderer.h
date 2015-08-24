#ifndef __SkinnedMeshRenderer_h__
#define __SkinnedMeshRenderer_h__

#include "Renderer.h"
#include "Mesh.h"

namespace Galaxy3D
{
    class SkinnedMeshRenderer : public Renderer
    {
    public:
        virtual ~SkinnedMeshRenderer() {}
        virtual void Render();
        void SetMesh(std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}

    private:
        std::shared_ptr<Mesh> m_mesh;
    };
}

#endif
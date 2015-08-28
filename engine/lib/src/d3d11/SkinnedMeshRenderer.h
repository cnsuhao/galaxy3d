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
        void SetMesh(const std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}
        void SetBones(const std::vector<std::shared_ptr<Transform>> &bones) {m_bones = bones;}

    private:
        std::shared_ptr<Mesh> m_mesh;
        std::vector<std::shared_ptr<Transform>> m_bones;
    };
}

#endif
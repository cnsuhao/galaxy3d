#ifndef __MeshRenderer_h__
#define __MeshRenderer_h__

#include "Renderer.h"
#include "Mesh.h"

namespace Galaxy3D
{
    class MeshRenderer : public Renderer
    {
        DECLARE_COM_CLASS(MeshRenderer, Component);

    public:
        virtual void DeepCopy(std::shared_ptr<Object> &source);
        void SetMesh(const std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}
        std::shared_ptr<Mesh> GetMesh() const {return m_mesh;}
        void RenderStaticBatch(const RenderBatch *batch, const RenderBatch *last_batch);

    protected:
        virtual void Render(int material_index);

    private:
        std::shared_ptr<Mesh> m_mesh;
    };
}

#endif
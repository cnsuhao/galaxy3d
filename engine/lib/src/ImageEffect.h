#ifndef __ImageEffect_h__
#define __ImageEffect_h__

#include "Component.h"
#include "Material.h"
#include "RenderTexture.h"

namespace Galaxy3D
{
    class Camera;

    class ImageEffect : public Component
    {
    public:
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
        bool IsOpaque() const {return m_opaque;}

    protected:
        std::shared_ptr<Material> m_material;
        bool m_opaque;

        void SetZBufferParams(const std::shared_ptr<Camera> &cam);
        void SetProjectionParams(const std::shared_ptr<Camera> &cam);
        void SetMainTexTexelSize(const std::shared_ptr<Texture> &tex);
        void SetOpaque(bool opaque) {m_opaque = opaque;}
    };
}

#endif
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

    protected:
        std::shared_ptr<Material> m_material;

        void SetZBufferParams(const std::shared_ptr<Camera> &cam);
        void SetMainTexTexelSize(const std::shared_ptr<Texture> &tex);
    };
}

#endif
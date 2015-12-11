#ifndef __ImageEffect_h__
#define __ImageEffect_h__

#include "Component.h"

namespace Galaxy3D
{
    class Material;
    class RenderTexture;

    class ImageEffect : public Component
    {
    public:
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);

    protected:
        std::shared_ptr<Material> m_material;
    };
}

#endif
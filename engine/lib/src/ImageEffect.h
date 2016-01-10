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
        ImageEffect():
            m_opaque(false)
        {}
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
        //
        // 摘要:
        //     ///
        //     Opaque image effects applied after opaque render, and before transpaent render
        //     ///
        bool IsOpaque() const {return m_opaque;}

    protected:
        std::shared_ptr<Material> m_material;
        bool m_opaque;

        void SetOpaque(bool opaque) {m_opaque = opaque;}
    };
}

#endif
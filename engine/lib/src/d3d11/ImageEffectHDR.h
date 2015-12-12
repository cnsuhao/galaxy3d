#ifndef __ImageEffectHDR_h__
#define __ImageEffectHDR_h__

#include "ImageEffect.h"

namespace Galaxy3D
{
    class ImageEffectHDR : public ImageEffect
    {
    public:
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
    };
}

#endif
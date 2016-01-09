#ifndef __ImageEffectHighlighting_h__
#define __ImageEffectHighlighting_h__

#include "ImageEffect.h"

namespace Galaxy3D
{
    class ImageEffectHighlighting : public ImageEffect
    {
    public:
        ImageEffectHighlighting()
        {}
        virtual void Start();
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);

    private:
    };
}

#endif
#ifndef __ImageEffectBloom_h__
#define __ImageEffectBloom_h__

#include "ImageEffect.h"

namespace Galaxy3D
{
    class ImageEffectBloom : public ImageEffect
    {
    public:
        ImageEffectBloom():
            m_threshold(0.4f),
            m_blur_iterations(2),
            m_blur_spread(3),
            m_intensity(2)
        {}
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
        void SetThreshold(float threshold) {m_threshold = threshold;}
        void SetBlurIterations(int iterations) {m_blur_iterations = iterations;}
        void SetBlurSpread(float spread) {m_blur_spread = spread;}
        void SetIntensity(float intensity) {m_intensity = intensity;}

    protected:
        virtual void Start();

    private:
        float m_threshold;
        int m_blur_iterations;
        float m_blur_spread;
        float m_intensity;
    };
}

#endif
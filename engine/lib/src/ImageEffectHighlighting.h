#ifndef __ImageEffectHighlighting_h__
#define __ImageEffectHighlighting_h__

#include "ImageEffect.h"

namespace Galaxy3D
{
    class ImageEffectHighlighting : public ImageEffect
    {
    public:
        ImageEffectHighlighting():
            m_blur_iterations(2),
            m_blur_min_spread(0.65f),
            m_blur_spread(0.25f),
            m_intensity(0.3f)
        {}
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
        void SetBlurIterations(int iterations) {m_blur_iterations = iterations;}
        void SetBlurMinSpread(float spread) {m_blur_min_spread = spread;}
        void SetBlurSpread(float spread) {m_blur_spread = spread;}
        void SetIntensity(float intensity) {m_intensity = intensity;}

    protected:
        virtual void Start();

    private:
        std::shared_ptr<Camera> m_camera;
        std::shared_ptr<Camera> m_camera_highlighting;
        std::shared_ptr<RenderTexture> m_render_texture_highlighting;
        int m_blur_iterations;
        float m_blur_min_spread;
        float m_blur_spread;
        float m_intensity;
    };
}

#endif
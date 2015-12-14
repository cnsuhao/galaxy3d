#ifndef __ImageEffectGlobalFog_h__
#define __ImageEffectGlobalFog_h__

#include "ImageEffect.h"

namespace Galaxy3D
{
    struct FogMode
    {
        enum Enum
        {
            Linear = 1,
            Exponential = 2,
            ExponentialSquared = 3
        };
    };

    class ImageEffectGlobalFog : public ImageEffect
    {
    public:
        ImageEffectGlobalFog():
            m_distance_fog_enable(true),
            m_use_radial_distance(false),
            m_start_distance(0),
            m_height_fog_enable(true),
            m_height(1.0f),
            m_height_density(2.0f),
            m_exclude_far_pixels(false),
            m_fog_mode(FogMode::Linear),
            m_linear_start(0),
            m_linear_end(300),
            m_fog_density(0.01f),
            m_fog_color(0.6f, 0.6f, 0.6f, 1)
        {}
        virtual void Start();
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
        void EnableDistance(bool enable) {m_distance_fog_enable = enable;}
        void UseRadialDistance(bool use) {m_use_radial_distance = use;}
        void SetStartDistance(float start) {m_start_distance = start;}
        void EnableHeight(bool enable) {m_height_fog_enable = enable;}
        void SetHeight(float height) {m_height = height;}
        void SetHeightDensity(float density) {m_height_density = density;}
        void ExcludeFarPixels(bool exclude) {m_exclude_far_pixels = exclude;}
        void SetFogMode(FogMode::Enum mode) {m_fog_mode = mode;}
        void SetLinearStart(float start) {m_linear_start = start;}
        void SetLinearEnd(float end) {m_linear_end = end;}
        void SetFogDensity(float density) {m_fog_density = density;}
        void SetFogColor(Color color) {m_fog_color = color;}

    private:
        bool m_distance_fog_enable;
        bool m_use_radial_distance;
        float m_start_distance;
        bool m_height_fog_enable;
        float m_height;
        float m_height_density;
        bool m_exclude_far_pixels;
        FogMode::Enum m_fog_mode;
        float m_linear_start;
        float m_linear_end;
        float m_fog_density;
        Color m_fog_color;
    };
}

#endif
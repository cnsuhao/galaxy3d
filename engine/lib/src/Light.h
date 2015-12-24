#ifndef __Light_h__
#define __Light_h__

#include "Component.h"
#include "Color.h"
#include "Matrix4x4.h"

namespace Galaxy3D
{
    struct LightType
    {
        enum Enum
        {
            Point,
            Directional,
            Spot,
        };
    };

    class Mesh;
    class Material;
    class RenderTexture;

    class Light : public Component
    {
    public:
        static const int CASCADE_SHADOW_COUNT = 3;
        static void DeferredShadingLights(std::shared_ptr<Material> &material);
        static std::list<Light *> GetLightsHasShadow();
        static void SetCascadeSplits(const std::vector<float> &splits);
        Light();
        ~Light();
        void SetType(LightType::Enum type) {m_type = type;}
        LightType::Enum GetType() const {return m_type;}
        void SetSpotAngle(float angle) {m_spot_angle = angle;}
        void SetRange(float range);
        void SetColor(const Color &color) {m_color = color;}
        Color GetColor() const {return m_color;}
        void SetIntensity(float intensity) {m_intensity = intensity;}
        float GetIntensity() const {return m_intensity;}
        void EnableShadow(bool enable) {m_shadow_enable = enable;}
        bool IsShadowEnable() const {return m_shadow_enable;}
        std::shared_ptr<RenderTexture> GetShadowMap();
        void PrepareForRenderShadowMap();
        const Matrix4x4 &GetViewProjectionMatrix() const {return m_view_projection_matrices[m_cascade_rendering_index];}
        const Matrix4x4 &GetProjectionMatrix() const {return m_projection_matrices[m_cascade_rendering_index];}
        const Matrix4x4 &GetViewMatrix() const {return m_view_matrices[m_cascade_rendering_index];}
        void SetShadowBias(float bias) {m_shadow_bias = bias;}
        void SetShadowStrength(float strength) {m_shadow_strength = strength;}
        void EnableCascade(bool enable) {m_cascade = enable;}
        bool IsCascade() const {return m_cascade;}
        void SetCascadeViewport(int index);

    private:
        static const int SHADOW_MAP_SIZE_W = 1536;
        static const int SHADOW_MAP_SIZE_H = 1024;
        static float CASCADE_SPLITS[CASCADE_SHADOW_COUNT];
        static const float SPOT_NEAR;
        static std::list<Light *> m_lights;
        static std::shared_ptr<Mesh> m_volume_sphere;
        static std::shared_ptr<Mesh> m_volume_cone;
        LightType::Enum m_type;
        float m_spot_angle;
        float m_range;
        Color m_color;
        float m_intensity;
        bool m_shadow_enable;
        float m_shadow_bias;
        float m_shadow_strength;
        bool m_cascade;
        int m_cascade_rendering_index;
        std::shared_ptr<RenderTexture> m_shadow_map;
        Matrix4x4 m_view_projection_matrices[CASCADE_SHADOW_COUNT];
        Matrix4x4 m_projection_matrices[CASCADE_SHADOW_COUNT];
        Matrix4x4 m_view_matrices[CASCADE_SHADOW_COUNT];

        static void CreateVolumeMeshIfNeeded();
        static void ShadingDirectionalLight(const Light *light, std::shared_ptr<Material> &material);
        void BuildViewProjectionMatrix();
        Matrix4x4 BuildDirectionalMatrix(float clip_near, float clip_far);
    };
}

#endif
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
        static void DeferredShadingLights(std::shared_ptr<Material> &material);
        static std::list<Light *> GetLightsHasShadow();
        Light();
        ~Light();
        void SetType(LightType::Enum type) {m_type = type;}
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
        const Matrix4x4 &GetViewProjectionMatrix() const {return m_view_projection_matrix;}

    private:
        static const int SHADOW_MAP_SIZE = 1024;
        static std::list<Light *> m_lights;
        static std::shared_ptr<Mesh> m_volume_sphere;
        static std::shared_ptr<Mesh> m_volume_cone;
        LightType::Enum m_type;
        float m_spot_angle;
        float m_range;
        Color m_color;
        float m_intensity;
        bool m_shadow_enable;
        std::shared_ptr<RenderTexture> m_shadow_map;
        Matrix4x4 m_view_projection_matrix;

        static void CreateVolumeMeshIfNeeded();
        static void ShadingDirectionalLight(const Light *light, std::shared_ptr<Material> &material, bool add);
        void BuildViewProjectionMatrix();
    };
}

#endif
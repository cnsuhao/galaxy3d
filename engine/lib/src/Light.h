#ifndef __Light_h__
#define __Light_h__

#include "Component.h"
#include "Color.h"

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

    class Light : public Component
    {
    public:
        static void DeferredShadingLights(std::shared_ptr<Material> &material);
        Light();
        ~Light();
        void SetType(LightType::Enum type) {m_type = type;}
        void SetSpotAngle(float angle) {m_spot_angle = angle;}
        void SetRange(float range);
        void SetColor(const Color &color) {m_color = color;}
        void SetIntensity(float intensity) {m_intensity = intensity;}

    private:
        static std::list<Light *> m_lights;
        static std::shared_ptr<Mesh> m_volume_sphere;
        static std::shared_ptr<Mesh> m_volume_cone;
        LightType::Enum m_type;
        float m_spot_angle;
        float m_range;
        Color m_color;
        float m_intensity;

        static void CreateVolumeMeshIfNeeded();
    };
}

#endif
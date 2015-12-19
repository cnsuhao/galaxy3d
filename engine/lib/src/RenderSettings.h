#ifndef __RenderSettings_h__
#define __RenderSettings_h__

#include "Color.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Light.h"

namespace Galaxy3D
{
    class RenderSettings
    {
    public:
        static Color light_ambient;
        static void SetGlobalDirectionalLight(std::shared_ptr<Light> &light) {m_light_directional = light;}
        static std::shared_ptr<Light> GetGlobalDirectionalLight();

    private:
        static std::shared_ptr<Light> m_light_directional;
    };
}

#endif
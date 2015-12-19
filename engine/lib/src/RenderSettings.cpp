#include "RenderSettings.h"
#include "GameObject.h"

namespace Galaxy3D
{
    Color RenderSettings::light_ambient(0, 0, 0, 0);
    std::shared_ptr<Light> RenderSettings::m_light_directional;

    std::shared_ptr<Light> RenderSettings::GetGlobalDirectionalLight()
    {
        if(!m_light_directional)
        {
            auto light = GameObject::Create("GlobalDirectionalLight")->AddComponent<Light>();
            light->GetTransform()->SetRotation(Quaternion::Euler(50, -30, 0));
            light->SetType(LightType::Directional);

            m_light_directional = light;
        }

        return m_light_directional;
    }
}
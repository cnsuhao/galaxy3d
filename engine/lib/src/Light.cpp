#include "Light.h"
#include "Mesh.h"
#include "GraphicsDevice.h"
#include "RenderSettings.h"
#include "RenderTexture.h"

namespace Galaxy3D
{
    // light attenuation : intensity = 1 / (C + L * d + Q * d * d)
    // use L = 0
    static const float LIGHT_CONSTANT_ATTENUATION = 1;
    static const float LIGHT_LINEAR_ATTENUATION = 0;
    static const float LIGHT_QUADRATIC_ATTENUATION = 1;
    static const float LIGHT_ATTENUATION_MIN = 0.001f;

    std::list<Light *> Light::m_lights;
    std::shared_ptr<Mesh> Light::m_volume_sphere;
    std::shared_ptr<Mesh> Light::m_volume_cone;

    Light::Light():
        m_type(LightType::Point),
        m_spot_angle(30),
        m_range(10),
        m_attenuation(0),
        m_color(1, 1, 1, 1),
        m_intensity(1)
    {
        ComputeAttenuation();
        m_lights.push_back(this);
    }

    Light::~Light()
    {
        m_lights.remove(this);
    }

    void Light::SetRange(float range)
    {
        m_range = range;
        ComputeAttenuation();
    }

    void Light::ComputeAttenuation()
    {
        if(m_range > 0)
        {
            // set L = 0
            m_attenuation = (1 / LIGHT_ATTENUATION_MIN - LIGHT_CONSTANT_ATTENUATION) / (m_range * m_range);
        }
        else
        {
            m_attenuation = 0;
        }
    }

    void Light::CreateVolumeMeshIfNeeded()
    {
        if(!m_volume_sphere)
        {
            m_volume_sphere = Mesh::CreateMeshSphere();
        }

        if(!m_volume_cone)
        {
            m_volume_cone = Mesh::CreateMeshCone();
        }
    }

    void Light::DeferredShadingLights(std::shared_ptr<Material> &material)
    {
        CreateVolumeMeshIfNeeded();

        auto camera = Camera::GetCurrent();
        auto vp = camera->GetViewProjectionMatrix();

        for(auto i : m_lights)
        {
            if(i->m_type == LightType::Point)
            {
                auto wvp = vp * Matrix4x4::TRS(i->GetTransform()->GetPosition(), Quaternion::Identity(), Vector3(1, 1, 1) * i->m_range);
                material->SetMatrix("WorldViewProjection", wvp);
                GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 1);
                
                material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                material->SetColor("LightColor", i->m_color * i->m_intensity);
                material->SetVector("LightAttenuation", i->m_range);
                GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 2);
            }
            else if(i->m_type == LightType::Spot)
            {
                GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 3);
            }
        }
    }
}
#include "Light.h"
#include "Mesh.h"
#include "GraphicsDevice.h"
#include "RenderSettings.h"
#include "RenderTexture.h"

namespace Galaxy3D
{
    std::list<Light *> Light::m_lights;
    std::shared_ptr<Mesh> Light::m_volume_sphere;
    std::shared_ptr<Mesh> Light::m_volume_cone;

    Light::Light():
        m_type(LightType::Point),
        m_spot_angle(30),
        m_range(10),
        m_color(1, 1, 1, 1),
        m_intensity(1)
    {
        m_lights.push_back(this);
    }

    Light::~Light()
    {
        m_lights.remove(this);
    }

    void Light::SetRange(float range)
    {
        m_range = range;
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
                GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 2);
            }
            else if(i->m_type == LightType::Spot)
            {
                GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 3);
            }
        }
    }
}
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
        FrustumBounds frustum(camera->GetViewProjectionMatrix());

        for(auto i : m_lights)
        {
            if(i->m_type == LightType::Point)
            {
                Vector3 pos = i->GetTransform()->GetPosition();
                float radius = i->m_range;

                if(frustum.ContainsSphere(pos, radius) != ContainsResult::Out)
                {
                    auto wvp = vp * Matrix4x4::TRS(pos, Quaternion::Identity(), Vector3(1, 1, 1) * radius);
                    material->SetMatrix("WorldViewProjection", wvp);
                    GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 1);

                    material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                    material->SetColor("LightColor", i->m_color * i->m_intensity);
                    GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 2);
                }
            }
            else if(i->m_type == LightType::Spot)
            {
                // 使用圆锥的外接球进行视锥剔除
                float tg = tanf(i->m_spot_angle * 0.5f * Mathf::Deg2Rad);
                float radius = i->m_range * 0.5f * (1 + tg * tg);
                Vector3 pos = i->GetTransform()->GetRotation() * (i->GetTransform()->GetPosition() + Vector3(0, 0, radius));

                if(frustum.ContainsSphere(pos, radius) != ContainsResult::Out)
                {
                    float scale_xy = i->m_range * tg;
                    auto wvp = vp * Matrix4x4::TRS(i->GetTransform()->GetPosition(), i->GetTransform()->GetRotation(), Vector3(scale_xy, scale_xy, i->m_range));
                    material->SetMatrix("WorldViewProjection", wvp);
                    GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 1);

                    material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                    material->SetColor("LightColor", i->m_color * i->m_intensity);
                    Vector3 spot_dir = (i->GetTransform()->GetRotation() * Vector3(0, 0, 1));
                    spot_dir.Normalize();
                    Vector4 spot_param = spot_dir;
                    spot_param.w = i->m_spot_angle * Mathf::Deg2Rad;
                    material->SetVector("SpotParam", spot_param);
                    GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 3);
                }
            }
        }
    }
}
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
        m_intensity(1),
        m_shadow_enable(false)
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

    std::shared_ptr<RenderTexture> Light::GetShadowMap()
    {
        if(m_shadow_enable)
        {
            if(!m_shadow_map)
            {
                m_shadow_map = RenderTexture::Create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, RenderTextureFormat::Depth, DepthBuffer::Depth_0, FilterMode::Bilinear);
            }
        }

        return m_shadow_map;
    }

    void Light::PrepareForRenderShadowMap()
    {
        auto shadow_map = GetShadowMap();
        auto camera = Camera::GetCurrent();

        camera->SetRenderTarget(shadow_map);

        BuildViewProjectionMatrix();
    }

    void Light::BuildViewProjectionMatrix()
    {
        auto camera = Camera::GetCurrent();


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

    std::list<Light *> Light::GetLightsHasShadow()
    {
        std::list<Light *> lights;

        for(auto i : m_lights)
        {
            if(i->m_shadow_enable)
            {
                lights.push_back(i);
            }
        }

        return lights;
    }

    void Light::ShadingDirectionalLight(const Light *light, std::shared_ptr<Material> &material, bool add)
    {
        auto camera = Camera::GetCurrent();

        material->SetVector("LightDirection", Vector4(light->GetTransform()->GetRotation() * Vector3(0, 0, 1)));
        material->SetColor("LightColor", light->GetColor() * light->GetIntensity());
        GraphicsDevice::GetInstance()->Blit(material->GetMainTexture(), camera->GetRenderTarget(), material, add ? 1 : 0);
    }

    void Light::DeferredShadingLights(std::shared_ptr<Material> &material)
    {
        CreateVolumeMeshIfNeeded();

        auto camera = Camera::GetCurrent();
        auto vp = camera->GetViewProjectionMatrix();
        FrustumBounds frustum(camera->GetViewProjectionMatrix());

        // shading global directional light first with blend off
        ShadingDirectionalLight(RenderSettings::GetGlobalDirectionalLight().get(), material, false);

        // shading other lights with blend add
        for(auto i : m_lights)
        {
            if(camera->IsCulling(i->GetGameObject()))
            {
                continue;
            }

            material->SetVector("LightRange", Vector4(i->m_range));

            if(i->m_type == LightType::Point)
            {
                Vector3 pos = i->GetTransform()->GetPosition();
                float radius = i->m_range;

                if(frustum.ContainsSphere(pos, radius) != ContainsResult::Out)
                {
                    material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                    material->SetColor("LightColor", i->m_color * i->m_intensity);

                    float distance = frustum.TestPlane(pos, 5);
                    if(fabs(distance) < radius)
                    {
                        // bound sphere cross with far plane, stencil test will get error result
                        // so draw full screen quad like directional light
                        GraphicsDevice::GetInstance()->Blit(material->GetMainTexture(), camera->GetRenderTarget(), material, 5);
                    }
                    else
                    {
                        auto wvp = vp * Matrix4x4::TRS(pos, Quaternion::Identity(), Vector3(1, 1, 1) * radius);
                        material->SetMatrix("WorldViewProjection", wvp);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 2);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 3);
                    }
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
                    Vector3 spot_dir = (i->GetTransform()->GetRotation() * Vector3(0, 0, 1));
                    spot_dir.Normalize();
                    Vector4 spot_param = spot_dir;
                    spot_param.w = i->m_spot_angle * Mathf::Deg2Rad;
                    material->SetVector("SpotParam", spot_param);
                    material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                    material->SetColor("LightColor", i->m_color * i->m_intensity);

                    float distance = frustum.TestPlane(pos, 5);
                    if(fabs(distance) < radius)
                    {
                        // bound sphere cross with far plane, stencil test will get error result
                        // so draw full screen quad like directional light
                        GraphicsDevice::GetInstance()->Blit(material->GetMainTexture(), camera->GetRenderTarget(), material, 6);
                    }
                    else
                    {
                        float scale_xy = i->m_range * tg;
                        auto wvp = vp * Matrix4x4::TRS(i->GetTransform()->GetPosition(), i->GetTransform()->GetRotation(), Vector3(scale_xy, scale_xy, i->m_range));
                        material->SetMatrix("WorldViewProjection", wvp);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 2);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 4);
                    }
                }
            }
            else if(i->m_type == LightType::Directional)
            {
                if(i != RenderSettings::GetGlobalDirectionalLight().get())
                {
                    ShadingDirectionalLight(i, material, true);
                }
            }
        }
    }
}
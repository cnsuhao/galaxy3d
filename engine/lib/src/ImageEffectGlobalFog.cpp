#include "ImageEffectGlobalFog.h"
#include "GameObject.h"
#include "Camera.h"

namespace Galaxy3D
{
    void ImageEffectGlobalFog::Start()
    {
        m_material = Material::Create("ImageEffect/GlobalFog");
        SetOpaque(true);
    }

    void ImageEffectGlobalFog::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        if(!m_distance_fog_enable && !m_height_fog_enable)
        {
            GraphicsDevice::GetInstance()->Blit(source, destination, std::shared_ptr<Material>(), 0);
        }

        auto cam = GetGameObject()->GetComponent<Camera>();
        auto camtr = cam->GetTransform();
        auto camPos= camtr->GetPosition();
        float FdotC = camPos.y - m_height;
        float paramK = (FdotC <= 0.0f ? 1.0f : 0.0f);
        float excludeDepth = m_exclude_far_pixels ? 1.0f : 2.0f;

        cam->SetFrustumCornersWS(m_material);
        m_material->SetVector("_CameraWS", camPos);
        m_material->SetVector("_HeightParams", Vector4(m_height, FdotC, paramK, m_height_density * 0.5f));
        m_material->SetVector("_DistanceParams", Vector4(-Mathf::Max(m_start_distance, 0.0f), excludeDepth, 0, 0));

        Vector4 sceneParams;
        bool  linear = (m_fog_mode == FogMode::Linear);
        float diff = linear ? m_linear_end - m_linear_start : 0.0f;
        float invDiff = fabs(diff) > 0.0001f ? 1.0f / diff : 0.0f;
        sceneParams.x = m_fog_density * 1.2011224087f; // density / sqrt(ln(2)), used by Exp2 fog mode
        sceneParams.y = m_fog_density * 1.4426950408f; // density / ln(2), used by Exp fog mode
        sceneParams.z = linear ? -invDiff : 0.0f;
        sceneParams.w = linear ? m_linear_end * invDiff : 0.0f;
        m_material->SetVector("_SceneFogParams", sceneParams);
        m_material->SetVector("_SceneFogMode", 
            Vector4(
                (float) m_fog_mode,
                m_use_radial_distance ? 1.0f : 0.0f,
                m_distance_fog_enable ? 1.0f : 0.0f,
                m_height_fog_enable ? 1.0f : 0.0f));
        m_material->SetColor("_SceneFogColor", m_fog_color);

        cam->SetZBufferParams(m_material);
        cam->SetProjectionParams(m_material);
        auto depth_texture = cam->GetDepthTexture();
        m_material->SetTexture("_CameraDepthTexture", depth_texture);
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }
}
#include "ImageEffectGlobalFog.h"
#include "GameObject.h"
#include "Camera.h"

namespace Galaxy3D
{
    void ImageEffectGlobalFog::Start()
    {
        m_material = Material::Create("ImageEffect/GlobalFog");
    }

    void ImageEffectGlobalFog::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        auto cam = GetGameObject()->GetComponent<Camera>();
        auto camtr = cam->GetTransform();
        float camNear = cam->GetClipNear();
        float camFar = cam->GetClipFar();
        float camFov = cam->GetFieldOfView();
        float camAspect = source->GetWidth() / (float) source->GetHeight();
        auto frustumCorners = Matrix4x4::Identity();
        float fovWHalf = camFov * 0.5f;

        Vector3 toRight = camtr->GetRight() * camNear * tan(fovWHalf * Mathf::Deg2Rad) * camAspect;
        Vector3 toTop = camtr->GetUp() * camNear * tan(fovWHalf * Mathf::Deg2Rad);

        Vector3 topLeft = (camtr->GetForward() * camNear - toRight + toTop);
        float camScale = topLeft.Magnitude() * camFar/camNear;
        topLeft.Normalize();
        topLeft *= camScale;

        Vector3 topRight = (camtr->GetForward() * camNear + toRight + toTop);
        topRight.Normalize();
        topRight *= camScale;

        Vector3 bottomRight = (camtr->GetForward() * camNear + toRight - toTop);
        bottomRight.Normalize();
        bottomRight *= camScale;

        Vector3 bottomLeft = (camtr->GetForward() * camNear - toRight - toTop);
        bottomLeft.Normalize();
        bottomLeft *= camScale;

        frustumCorners.SetRow(0, topLeft);
        frustumCorners.SetRow(1, topRight);
        frustumCorners.SetRow(2, bottomRight);
        frustumCorners.SetRow(3, bottomLeft);

        auto camPos= camtr->GetPosition();
        float FdotC = camPos.y - m_height;
        float paramK = (FdotC <= 0.0f ? 1.0f : 0.0f);
        float excludeDepth = m_exclude_far_pixels ? 1.0f : 2.0f;
        m_material->SetMatrix("_FrustumCornersWS", frustumCorners);
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

        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }
}
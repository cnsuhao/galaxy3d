#include "ImageEffectHighlighting.h"
#include "GameObject.h"
#include "Camera.h"
#include "Layer.h"
#include "LayerMask.h"

namespace Galaxy3D
{
    void ImageEffectHighlighting::Start()
    {
        m_camera = GetGameObject()->GetComponent<Camera>();

        m_render_texture_highlighting = RenderTexture::Create(m_camera->GetPixelWidth(), m_camera->GetPixelHeight(), RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Bilinear);

        m_camera_highlighting = GameObject::Create("camera_highlighting")->AddComponent<Camera>();
        m_camera_highlighting->GetTransform()->SetParent(m_camera->GetTransform());
        m_camera_highlighting->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
        m_camera_highlighting->GetTransform()->SetLocalRotation(Quaternion::Identity());
        m_camera_highlighting->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

        m_camera_highlighting->SetClearFlags(CameraClearFlags::SolidColor);
        m_camera_highlighting->SetClearColor(Color(0, 0, 0, 0));
        m_camera_highlighting->SetDepth(m_camera->GetDepth() - 1);
        m_camera_highlighting->SetCullingMask(LayerMask::GetMask(Layer::Highlighting));
        m_camera_highlighting->SetFieldOfView(m_camera->GetFieldOfView());
        m_camera_highlighting->SetClipPlane(m_camera->GetClipNear(), m_camera->GetClipFar());
        m_camera_highlighting->SetRect(m_camera->GetRect());
        m_camera_highlighting->SetRenderTexture(m_render_texture_highlighting);

        m_material = Material::Create("ImageEffect/Highlighting");
    }

    void ImageEffectHighlighting::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        auto quarter_w = m_render_texture_highlighting->GetWidth() / 4;
        auto quarter_h = m_render_texture_highlighting->GetHeight() / 4;
        auto format = m_render_texture_highlighting->GetFormat();
        auto rt_quarter = RenderTexture::GetTemporary(quarter_w, quarter_h, format, DepthBuffer::Depth_0);

        // downsample
        m_material->SetMainTexTexelSize(m_render_texture_highlighting);
        m_material->SetVector("OffsetScale", 1.0f);
        m_material->SetVector("Intensity", m_intensity);
        GraphicsDevice::GetInstance()->Blit(m_render_texture_highlighting, rt_quarter, m_material, 0);
  
        // blur
        m_material->SetMainTexTexelSize(rt_quarter);
        m_blur_iterations = Mathf::Min(Mathf::Max(m_blur_iterations, 1), 10);
        for(int i=0; i<m_blur_iterations; i++)
        {
            float off = m_blur_min_spread + i * m_blur_spread;
            m_material->SetVector("OffsetScale", off);

            auto blur = RenderTexture::GetTemporary(quarter_w, quarter_h, format, DepthBuffer::Depth_0);
            GraphicsDevice::GetInstance()->Blit(rt_quarter, blur, m_material, 0);
            RenderTexture::ReleaseTemporary(rt_quarter);
            rt_quarter = blur;
        }

        // compose
        m_material->SetTexture("TexHighlight", m_render_texture_highlighting);
        m_material->SetTexture("TexBlur", rt_quarter);

        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 1);
        
        RenderTexture::ReleaseTemporary(rt_quarter);
    }
}
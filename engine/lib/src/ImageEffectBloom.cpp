#include "ImageEffectBloom.h"

namespace Galaxy3D
{
    void ImageEffectBloom::Start()
    {
        m_material = Material::Create("ImageEffect/Bloom");
    }

    void ImageEffectBloom::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        auto rtW2 = source->GetWidth()/2;
        auto rtH2 = source->GetHeight()/2;
        auto rtW4 = source->GetWidth()/4;
        auto rtH4 = source->GetHeight()/4;
        auto rtFormat = source->GetFormat();
        float widthOverHeight = (1.0f * source->GetWidth()) / (1.0f * source->GetHeight());
        float oneOverBaseSize = 1.0f / 512.0f;
        std::shared_ptr<Material> mat_empty;

        // downsample
        auto halfRezColorDown = RenderTexture::GetTemporary(rtW2, rtH2, rtFormat, DepthBuffer::Depth_0);
        auto quarterRezColor = RenderTexture::GetTemporary(rtW4, rtH4, rtFormat, DepthBuffer::Depth_0);
        GraphicsDevice::GetInstance()->Blit(source, halfRezColorDown, mat_empty, 0);

        SetMainTexTexelSize(halfRezColorDown);
        GraphicsDevice::GetInstance()->Blit(halfRezColorDown, quarterRezColor, m_material, 0);
        RenderTexture::ReleaseTemporary(halfRezColorDown);

        // bright
        auto secondQuarterRezColor = RenderTexture::GetTemporary(rtW4, rtH4, rtFormat, DepthBuffer::Depth_0);
        m_material->SetVector("_Threshhold", Vector4(1, 1, 1, 1) * m_threshold);
        GraphicsDevice::GetInstance()->Blit(quarterRezColor, secondQuarterRezColor, m_material, 1);

        // blur
        m_blur_iterations = Mathf::Min(Mathf::Max(m_blur_iterations, 1), 10);
        for(int i=0; i<m_blur_iterations; i++)
        {
            float spreadForPass = (1.0f + (i * 0.25f)) * m_blur_spread;

            // vertical blur
            auto blur4 = RenderTexture::GetTemporary(rtW4, rtH4, rtFormat, DepthBuffer::Depth_0);
            m_material->SetVector("_Offsets", Vector4(0, spreadForPass * oneOverBaseSize, 0, 0));
            GraphicsDevice::GetInstance()->Blit(secondQuarterRezColor, blur4, m_material, 2);
            RenderTexture::ReleaseTemporary(secondQuarterRezColor);
            secondQuarterRezColor = blur4;

            // horizontal blur
            blur4 = RenderTexture::GetTemporary(rtW4, rtH4, rtFormat, DepthBuffer::Depth_0);
            m_material->SetVector("_Offsets", Vector4((spreadForPass / widthOverHeight) * oneOverBaseSize, 0, 0, 0));
            GraphicsDevice::GetInstance()->Blit(secondQuarterRezColor, blur4, m_material, 2);
            RenderTexture::ReleaseTemporary(secondQuarterRezColor);
            secondQuarterRezColor = blur4;
        }

        // blend add
        m_material->SetVector("_Intensity", m_intensity);
        m_material->SetTexture("_ColorBuffer", source);
        GraphicsDevice::GetInstance()->Blit(secondQuarterRezColor, destination, m_material, 3);

        RenderTexture::ReleaseTemporary(quarterRezColor);
        RenderTexture::ReleaseTemporary(secondQuarterRezColor);
    }
}
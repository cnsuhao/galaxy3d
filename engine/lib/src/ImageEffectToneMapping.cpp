#include "ImageEffectToneMapping.h"
#include "Mathf.h"

namespace Galaxy3D
{
    void ImageEffectToneMapping::Start()
    {
        m_material = Material::Create("ImageEffect/ToneMapping");
    }

    void ImageEffectToneMapping::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        std::shared_ptr<Material> mat_empty;

        bool new_texture_1 = NewAdaptiveTexture();

        auto adaptive_texture = RenderTexture::GetTemporary(m_adaptive_texture_size, m_adaptive_texture_size, RenderTextureFormat::RGBAHalf, DepthBuffer::Depth_0);
        // down sample
        GraphicsDevice::GetInstance()->Blit(source, adaptive_texture, mat_empty, 0);
        
        int lum_tex_count = (int) Mathf::Log2((float) m_adaptive_texture_size);
        int div = 2;
        std::vector<std::shared_ptr<RenderTexture>> rts(lum_tex_count);
        for(int i=0; i<lum_tex_count; i++)
        {
            int size = m_adaptive_texture_size/div;
            rts[i] = RenderTexture::GetTemporary(size, size, RenderTextureFormat::RGHalf, DepthBuffer::Depth_0);
            div *= 2;
        }

        m_material->SetMainTexTexelSize(adaptive_texture);

        // log
        GraphicsDevice::GetInstance()->Blit(adaptive_texture, rts[0], m_material, 0);

        // down sample lum
        for(int i=0; i<lum_tex_count-1; i++)
        {
            GraphicsDevice::GetInstance()->Blit(rts[i], rts[i + 1], mat_empty, 0);
        }
        auto lumRt = rts[lum_tex_count - 1];
        
        m_adaption_speed = Mathf::Max(m_adaption_speed, 0.001f);
        m_material->SetVector("_AdaptionSpeed", m_adaption_speed);

        // exp
        GraphicsDevice::GetInstance()->Blit(lumRt, m_adaptive_texture_1, m_material, new_texture_1 ? 1 : 2);

        m_middle_gray = Mathf::Max(m_middle_gray, 0.001f);
        m_material->SetVector("_HdrParams", Vector4(m_middle_gray, m_middle_gray, m_middle_gray, m_white * m_white));
        m_material->SetTexture("_SmallTex", m_adaptive_texture_1);

        // final
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 3);

        RenderTexture::ReleaseTemporary(adaptive_texture);
        for(int i=0; i<lum_tex_count; i++)
        {
            RenderTexture::ReleaseTemporary(rts[i]);
        }
    }

    bool ImageEffectToneMapping::NewAdaptiveTexture()
    {
        if(m_adaptive_texture_1)
        {
            return false;
        }

        m_adaptive_texture_1 = RenderTexture::Create(
            1,
            1,
            RenderTextureFormat::RGHalf,
            DepthBuffer::Depth_0,
            FilterMode::Bilinear);
        m_adaptive_texture_1->MarkKeepBuffer(true);
        return true;
    }
}
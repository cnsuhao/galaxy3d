#include "ImageEffect.h"
#include "Camera.h"

namespace Galaxy3D
{
    void ImageEffect::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }

    void ImageEffect::SetMainTexTexelSize(const std::shared_ptr<Texture> &tex)
    {
        m_material->SetVector("_MainTex_TexelSize", Vector4(1.0f / tex->GetWidth(), 1.0f / tex->GetHeight(), (float) tex->GetWidth(), (float) tex->GetHeight()));
    }
}
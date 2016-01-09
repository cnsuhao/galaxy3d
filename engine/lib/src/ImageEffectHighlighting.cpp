#include "ImageEffectHighlighting.h"

namespace Galaxy3D
{
    void ImageEffectHighlighting::Start()
    {
        //m_material = Material::Create("ImageEffect/Bloom");
    }

    void ImageEffectHighlighting::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }
}
#include "ImageEffectHDR.h"

namespace Galaxy3D
{
    void ImageEffectHDR::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }
}
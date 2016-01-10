#include "ImageEffect.h"
#include "Camera.h"

namespace Galaxy3D
{
    void ImageEffect::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }
}
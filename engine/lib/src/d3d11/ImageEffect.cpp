#include "ImageEffect.h"
#include "Camera.h"

namespace Galaxy3D
{
    void ImageEffect::OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination)
    {
        GraphicsDevice::GetInstance()->Blit(source, destination, m_material, 0);
    }

    void ImageEffect::SetZBufferParams(const std::shared_ptr<Camera> &cam)
    {
        float cam_far = cam->GetClipFar();
        float cam_near = cam->GetClipNear();

#if defined(WINPC) || defined(WINPHONE)
        float zx = (1.0f - cam_far / cam_near) / 2;
        float zy = (1.0f + cam_far / cam_near) / 2;
#else
        float zx = (1.0f - cam_far / cam_near);
        float zy = (cam_far / cam_near);
#endif

        m_material->SetVector("_ZBufferParams", Vector4(zx, zy, zx / cam_far, zy / cam_near));
    }

    void ImageEffect::SetMainTexTexelSize(const std::shared_ptr<Texture> &tex)
    {
        m_material->SetVector("_MainTex_TexelSize", Vector4(1.0f / tex->GetWidth(), 1.0f / tex->GetHeight(), (float) tex->GetWidth(), (float) tex->GetHeight()));
    }
}
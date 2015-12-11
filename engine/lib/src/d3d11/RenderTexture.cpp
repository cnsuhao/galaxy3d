#include "RenderTexture.h"

namespace Galaxy3D
{
    std::shared_ptr<RenderTexture> RenderTexture::CreateRenderTexture(
        int width,
        int height,
        DepthBuffer::Enum depth,
        RenderTextureFormat::Enum format,
        FilterMode::Enum filter_mode,
        TextureWrapMode::Enum wrap_mode)
    {
        auto texture = std::shared_ptr<RenderTexture>(new RenderTexture(width, height, depth, format, filter_mode, wrap_mode));
        texture->Create();

        return texture;
    }

    RenderTexture::~RenderTexture()
    {
        SAFE_RELEASE(m_render_target_view);
        SAFE_RELEASE(m_depth_stencil_view);
        SAFE_RELEASE(m_shader_resource_view);
        SAFE_RELEASE(m_sampler_state);
    }

    void RenderTexture::Create()
    {

    }
}
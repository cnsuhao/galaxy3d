#include "RenderTexture.h"

namespace Galaxy3D
{
    std::shared_ptr<RenderTexture> RenderTexture::CreateRenderTexture(
        int width,
        int height,
        RenderTextureFormat::Enum format,
        DepthBuffer::Enum depth,
        FilterMode::Enum filter_mode,
        TextureWrapMode::Enum wrap_mode)
    {
        auto texture = std::shared_ptr<RenderTexture>(new RenderTexture(width, height, format, depth, filter_mode, wrap_mode));
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
        auto device = GraphicsDevice::GetInstance()->GetDevice();

        if(m_format == RenderTextureFormat::RGBA32 || m_format == RenderTextureFormat::RGBAFloat)
        {
            DXGI_FORMAT fmt;
            if(m_format == RenderTextureFormat::RGBA32)
            {
                fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            else if(m_format == RenderTextureFormat::RGBAFloat)
            {
                fmt = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }

            D3D11_TEXTURE2D_DESC dtd =
            {
                m_width,//UINT Width;
                m_height,//UINT Height;
                1,//UINT MipLevels;
                1,//UINT ArraySize;
                fmt,//DXGI_FORMAT Format;
                1, 0,//DXGI_SAMPLE_DESC SampleDesc;
                D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
                D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
                0,//UINT CPUAccessFlags;
                0//UINT MiscFlags;
            };

            ID3D11Texture2D *texture = 0;
            device->CreateTexture2D(&dtd, NULL, &texture);
            device->CreateRenderTargetView(texture, NULL, &m_render_target_view);
            device->CreateShaderResourceView(texture, NULL, &m_shader_resource_view);
            texture->Release();
        }

        if(m_format != RenderTextureFormat::Depth && m_depth != DepthBuffer::Depth_0)
        {
            DXGI_FORMAT fmt;
            if(m_depth == DepthBuffer::Depth_16)
            {
                fmt = DXGI_FORMAT_D16_UNORM;
            }
            else if(m_depth == DepthBuffer::Depth_24)
            {
                fmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
            }

            D3D11_TEXTURE2D_DESC dtd_depth =
            {
                m_width,//UINT Width;
                m_height,//UINT Height;
                1,//UINT MipLevels;
                1,//UINT ArraySize;
                fmt,//DXGI_FORMAT Format;
                1, 0,//DXGI_SAMPLE_DESC SampleDesc;
                D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
                D3D11_BIND_DEPTH_STENCIL,//UINT BindFlags;
                0,//UINT CPUAccessFlags;
                0//UINT MiscFlags;    
            };

            ID3D11Texture2D *depth_texture = 0;
            device->CreateTexture2D(&dtd_depth, NULL, &depth_texture);
            device->CreateDepthStencilView(depth_texture, NULL, &m_depth_stencil_view);
            depth_texture->Release();
        }

        // create sampler states
        D3D11_SAMPLER_DESC dsd;
        ZeroMemory(&dsd, sizeof(dsd));
        dsd.Filter = Texture::FILTER_MODES[m_filter_mode];
        dsd.AddressU = Texture::ADDRESS_MODES[m_wrap_mode];
        dsd.AddressV = Texture::ADDRESS_MODES[m_wrap_mode];
        dsd.AddressW = Texture::ADDRESS_MODES[m_wrap_mode];
        dsd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        dsd.MinLOD = 0;
        dsd.MaxLOD = D3D11_FLOAT32_MAX;

        device->CreateSamplerState(&dsd, &m_sampler_state);
    }
}
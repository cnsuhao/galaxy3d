#ifndef __RenderTexture_h__
#define __RenderTexture_h__

#include "Texture.h"
#include "GraphicsDevice.h"

namespace Galaxy3D
{
    struct DepthBuffer
    {
        enum Enum
        {
            Depth_0,
            Depth_16,
            //          Depth_24,
        };
    };

    struct RenderTextureFormat
    {
        enum Enum
        {
            ARGB32 = 0,			//Color render texture format, 8 bits per channel
            Depth = 1,			//A depth render texture format
            //			ARGBHalf = 2,		//Color render texture format, 16 bit floating point per channel
            //			RGB565 = 4,
            //			ARGB4444 = 5,
            //			ARGB1555 = 6,
            //			Default = 7,
            //			DefaultHDR = 9,
            ARGBFloat = 11,		//Color render texture format, 32 bit floating point per channel
            //			RGFloat = 12,
            //			RGHalf = 13,
            //			RFloat = 14,		//Scalar (R) render texture format, 32 bit floating point
            //			RHalf = 15,
            //			R8 = 16,			//Scalar (R) render texture format, 8 bit fixed point
            //			ARGBInt = 17,		//Four channel (ARGB) render texture format, 32 bit signed integer per channel
            //			RGInt = 18,
            //			RInt = 19,
        };
    };

    class RenderTexture : public Texture
    {
    public:
        static std::shared_ptr<RenderTexture> CreateRenderTexture(
            int width,
            int height,
            DepthBuffer::Enum depth,
            RenderTextureFormat::Enum format,
            FilterMode::Enum filter_mode = FilterMode::Bilinear,
            TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
        virtual ~RenderTexture();

    private:
        DepthBuffer::Enum m_depth;
        RenderTextureFormat::Enum m_format;
        ID3D11RenderTargetView *m_render_target_view;
        ID3D11DepthStencilView *m_depth_stencil_view;
        ID3D11ShaderResourceView *m_shader_resource_view;
        ID3D11SamplerState *m_sampler_state;

        RenderTexture(
            int width, int height,
            DepthBuffer::Enum depth, RenderTextureFormat::Enum format,
            FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode) :
            Texture(width, height, filter_mode, wrap_mode),
            m_depth(depth),
            m_format(format),
            m_render_target_view(NULL),
            m_depth_stencil_view(NULL),
            m_shader_resource_view(NULL),
            m_sampler_state(NULL)
        {}
        void Create();
    };
}

#endif
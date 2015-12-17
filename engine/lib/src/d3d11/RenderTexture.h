#ifndef __RenderTexture_h__
#define __RenderTexture_h__

#include "Texture.h"
#include "GraphicsDevice.h"

namespace Galaxy3D
{
    struct RenderTextureFormat
    {
        enum Enum
        {
            RGBA32 = 0,			//Color render texture format, 8 bits per channel
            Depth = 1,			//A depth render texture format
            RGBAHalf = 2,		//Color render texture format, 16 bit floating point per channel
            //			RGB565 = 4,
            //			ARGB4444 = 5,
            //			ARGB1555 = 6,
            //			Default = 7,
            //			DefaultHDR = 9,
            //          RGBAFloat = 11,		//Color render texture format, 32 bit floating point per channel
            //          RGFloat = 12,
            RGHalf = 13,
            RFloat = 14,		//Scalar (R) render texture format, 32 bit floating point
            //			RHalf = 15,
            //			R8 = 16,			//Scalar (R) render texture format, 8 bit fixed point
            //			ARGBInt = 17,		//Four channel (ARGB) render texture format, 32 bit signed integer per channel
            //			RGInt = 18,
            //			RInt = 19,
        };
    };

    struct DepthBuffer
    {
        enum Enum
        {
            Depth_0,
            Depth_16,
            Depth_24,
        };
    };

    class RenderTexture : public Texture
    {
    public:
        static std::shared_ptr<RenderTexture> Create(
            int width,
            int height,
            RenderTextureFormat::Enum format,
            DepthBuffer::Enum depth,
            FilterMode::Enum filter_mode = FilterMode::Point,
            TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
        static std::shared_ptr<RenderTexture> GetTemporary(
            int width,
            int height,
            RenderTextureFormat::Enum format,
            DepthBuffer::Enum depth);
        static void ReleaseTemporary(const std::shared_ptr<RenderTexture> &temp);
        static void SwapColorBuffer(std::shared_ptr<RenderTexture> &t1, std::shared_ptr<RenderTexture> &t2);
        RenderTexture(int w, int h, ID3D11RenderTargetView *render_target_view, ID3D11DepthStencilView *depth_stencil_view):
            Texture(w, h, FilterMode::Point, TextureWrapMode::Clamp),
            m_format(RenderTextureFormat::RGBA32),
            m_depth(DepthBuffer::Depth_24),
            m_keep_buffer(false),
            m_is_depth_shader_resource_view(false),
            m_render_target_view(render_target_view),
            m_depth_stencil_view(depth_stencil_view),
            m_shader_resource_view_color(NULL),
            m_shader_resource_view_depth(NULL),
            m_sampler_state(NULL)
        {
            m_render_target_view->AddRef();
            m_depth_stencil_view->AddRef();
        }
        virtual ~RenderTexture();
        RenderTextureFormat::Enum GetFormat() const {return m_format;}
        DepthBuffer::Enum GetDepth() const {return m_depth;}
        ID3D11RenderTargetView *GetRenderTargetView() const {return m_render_target_view;}
        ID3D11DepthStencilView *GetDepthStencilView() const {return m_depth_stencil_view;}
        ID3D11ShaderResourceView *GetShaderResourceViewColor() const {return m_shader_resource_view_color;}
        ID3D11ShaderResourceView *GetShaderResourceViewDepth() const {return m_shader_resource_view_depth;}
        ID3D11SamplerState *GetSamplerState() const {return m_sampler_state;}
        void MarkKeepBuffer(bool keep) {m_keep_buffer = keep;}
        bool IsKeepBuffer() const {return m_keep_buffer;}

    private:
        static std::list<std::shared_ptr<RenderTexture>> m_textures_idle;
        static std::list<std::shared_ptr<RenderTexture>> m_textures_using;
        RenderTextureFormat::Enum m_format;
        DepthBuffer::Enum m_depth;
        bool m_keep_buffer;
        bool m_is_depth_shader_resource_view;
        ID3D11RenderTargetView *m_render_target_view;
        ID3D11DepthStencilView *m_depth_stencil_view;
        ID3D11ShaderResourceView *m_shader_resource_view_color;
        ID3D11ShaderResourceView *m_shader_resource_view_depth;
        ID3D11SamplerState *m_sampler_state;

        RenderTexture(
            int width, int height,
            RenderTextureFormat::Enum format, DepthBuffer::Enum depth,
            FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode) :
            Texture(width, height, filter_mode, wrap_mode),
            m_format(format),
            m_depth(depth),
            m_keep_buffer(false),
            m_is_depth_shader_resource_view(false),
            m_render_target_view(NULL),
            m_depth_stencil_view(NULL),
            m_shader_resource_view_color(NULL),
            m_shader_resource_view_depth(NULL),
            m_sampler_state(NULL)
        {}
        void Create();
    };
}

#endif
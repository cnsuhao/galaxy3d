#ifndef __GraphicsDevice_h__
#define __GraphicsDevice_h__

#include "CameraClearFlags.h"
#include "Color.h"
#include <d3d11.h>
#include <memory>
#include <vector>

#define SAFE_RELEASE(p) \
	do { \
		if(p != NULL) { \
			p->Release(); \
			p = NULL; \
		} \
	}while(false)

namespace Galaxy3D
{
    class RenderTexture;
    class Texture;
    class Material;
    class Mesh;
    class Matrix4x4;
    struct VertexShader;

    struct BufferUsage
    {
        enum Enum
        {
            StaticDraw,
            DynamicDraw
        };
    };

    struct BufferType
    {
        enum Enum
        {
            Vertex,
            Index
        };
    };

    struct IndexType
    {
        enum Enum
        {
            UShort,
            UInt
        };
    };

    struct BufferObject
    {
        void *buffer;

        BufferObject():buffer(NULL) {}
    };

	class GraphicsDevice
	{
#ifdef WINPHONE
    public:
        void SetDeviceResources(ID3D11Device *device, ID3D11DeviceContext *context)
        {
            SAFE_RELEASE(m_d3d_device);
            m_d3d_device = device;
            m_d3d_device->AddRef();
            SAFE_RELEASE(m_immediate_context);
            m_immediate_context = context;
            m_immediate_context->AddRef();
        }

        void SetWindowSizeDependentResources(
            IDXGISwapChain *swap_chain,
            ID3D11RenderTargetView *render_target,
            ID3D11DepthStencilView *depth_stencil)
        {
            SAFE_RELEASE(m_swap_chain);
            m_swap_chain = swap_chain;
            m_swap_chain->AddRef();
            SAFE_RELEASE(m_render_target_view);
            m_render_target_view = render_target;
            m_render_target_view->AddRef();
            SAFE_RELEASE(m_depth_stencil_view);
            m_depth_stencil_view = depth_stencil;
            m_depth_stencil_view->AddRef();
        }

		void ResetSizeDependentResources()
		{
			m_screen_buffer.reset();
			SAFE_RELEASE(m_swap_chain);
			SAFE_RELEASE(m_render_target_view);
			SAFE_RELEASE(m_depth_stencil_view);
		}
#endif

	public:
		static GraphicsDevice *GetInstance();
        static void Done();
		~GraphicsDevice();
		void Init(void *param);
		void ClearShaderResources();
		ID3D11Device *GetDevice() const {return m_d3d_device;}
		IDXGISwapChain *GetSwapChain() const {return m_swap_chain;}
		ID3D11DeviceContext *GetDeviceContext() const {return m_immediate_context;}
        std::weak_ptr<RenderTexture> GetScreenBuffer() const {return m_screen_buffer;}
        void Blit(const std::shared_ptr<Texture> &source, const std::shared_ptr<RenderTexture> &destination, const std::shared_ptr<Material> &material, int pass);
        void DrawMeshNow(const std::shared_ptr<Mesh> &mesh, int sub_mesh_index, const std::shared_ptr<Material> &material, int pass_index);
        void SetViewport(int left, int top, int width, int height);
        void Present();
        void SetRenderTargets(const std::vector<std::shared_ptr<RenderTexture>> &color_buffers, const std::shared_ptr<RenderTexture> &depth_stencil_buffer);
        void ClearRenderTarget(CameraClearFlags::Enum clear_flags, const Color &color, float depth, int stencil);
        BufferObject CreateBufferObject(void *data, int size, BufferUsage::Enum usage, BufferType::Enum type);
        void UpdateBufferObject(BufferObject &bo, void *data, int size);
        void ReleaseBufferObject(BufferObject &bo);
        void SetInputLayout(VertexShader *shader);
        void SetVertexBuffer(BufferObject &bo, VertexShader *shader);
        void SetIndexBuffer(BufferObject &bo, IndexType::Enum bits);
        void DrawIndexed(int count, int offset);
		bool IsRenderTargetScreen() const {return m_render_target_view_current == m_render_target_view;}

	private:
		ID3D11Device *m_d3d_device;
		ID3D11DeviceContext *m_immediate_context;
        IDXGISwapChain *m_swap_chain;
		ID3D11RenderTargetView *m_render_target_view;
		ID3D11RenderTargetView *m_render_target_view_current;
		ID3D11DepthStencilView *m_depth_stencil_view;
        std::shared_ptr<RenderTexture> m_screen_buffer;
        std::shared_ptr<Mesh> m_blit_mesh;
        std::shared_ptr<Material> m_blit_mat;
        std::vector<std::weak_ptr<RenderTexture>> m_color_buffers_ref;
        std::weak_ptr<RenderTexture> m_depth_stencil_buffer_ref;

        GraphicsDevice();
        void CreateBlitMeshIfNeeded();
        void CreateBlitMaterialIfNeeded();
	};
}

#endif
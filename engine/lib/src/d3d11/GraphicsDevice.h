#ifndef __GraphicsDevice_h__
#define __GraphicsDevice_h__

#include <d3d11.h>
#include <memory>

namespace Galaxy3D
{
    class RenderTexture;

	class GraphicsDevice
	{
	public:
		static GraphicsDevice *GetInstance();
		GraphicsDevice();
		~GraphicsDevice();
		void Init(void *param);
		void ClearShaderResources();
		ID3D11Device *GetDevice() const {return m_d3d_device;}
		IDXGISwapChain *GetSwapChain() const {return m_swap_chain;}
		ID3D11DeviceContext *GetDeviceContext() const {return m_immediate_context;}
        std::shared_ptr<RenderTexture> GetScreenBuffer() const {return m_screen_buffer;}

	private:
		ID3D11Device *m_d3d_device;
		IDXGISwapChain *m_swap_chain;
		ID3D11DeviceContext *m_immediate_context;
		ID3D11RenderTargetView *m_render_target_view;
		ID3D11Texture2D *m_depth_stencil_texture;
		ID3D11DepthStencilView *m_depth_stencil_view;
        std::shared_ptr<RenderTexture> m_screen_buffer;
	};
}

#define SAFE_RELEASE(p) \
	do { \
		if(p != NULL) { \
			p->Release(); \
			p = NULL; \
		} \
	}while(false)

#endif
#include "GraphicsDevice.h"
#include "Screen.h"

namespace Galaxy3D
{
	GraphicsDevice *GraphicsDevice::GetInstance()
	{
		static GraphicsDevice device;
		return &device;
	}

	GraphicsDevice::GraphicsDevice():
		m_d3d_device(NULL),
		m_swap_chain(NULL),
		m_immediate_context(NULL),
		m_render_target_view(NULL),
		m_depth_stencil_texture(NULL),
		m_depth_stencil_view(NULL)
	{
	}

	GraphicsDevice::~GraphicsDevice()
	{
		SAFE_RELEASE(m_depth_stencil_view);
		SAFE_RELEASE(m_depth_stencil_texture);
		SAFE_RELEASE(m_render_target_view);
		SAFE_RELEASE(m_immediate_context);
		SAFE_RELEASE(m_swap_chain);
		SAFE_RELEASE(m_d3d_device);
	}

	void GraphicsDevice::Init(void *param)
	{
		HRESULT hr = S_OK;

		UINT flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG
		flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HWND hwnd = (HWND) param;

		//device and swap chain
		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			//D3D_DRIVER_TYPE_WARP,
			//D3D_DRIVER_TYPE_REFERENCE,
		};
		int numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			//D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		int numFeatureLevels = ARRAYSIZE(featureLevels);

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = Screen::GetWidth();
		sd.BufferDesc.Height = Screen::GetHeight();
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for(int i=0; i<numDriverTypes; i++)
		{
			auto driverType = driverTypes[i];
			D3D_FEATURE_LEVEL level;
			hr = D3D11CreateDeviceAndSwapChain(
				NULL, driverType, NULL, flag,
				featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &m_swap_chain, &m_d3d_device, &level, &m_immediate_context);
			if(SUCCEEDED(hr))
				break;
		}
		if(FAILED(hr))
			return;

		//defaut render target 
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer);
		if(FAILED(hr))
			return;

		hr = m_d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &m_render_target_view);
		pBackBuffer->Release();
		if(FAILED(hr))
			return;

		//set primitive topology
		m_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//depth stencil texture
        D3D11_TEXTURE2D_DESC dtd =
        {
            Screen::GetWidth(),//UINT Width;
            Screen::GetHeight(),//UINT Height;
            1,//UINT MipLevels;
            1,//UINT ArraySize;
            DXGI_FORMAT_D24_UNORM_S8_UINT,//DXGI_FORMAT Format;
            1, 0,//DXGI_SAMPLE_DESC SampleDesc;
            D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
            D3D11_BIND_DEPTH_STENCIL,//UINT BindFlags;
            0,//UINT CPUAccessFlags;
            0//UINT MiscFlags;
        };
		m_d3d_device->CreateTexture2D(&dtd, NULL, &m_depth_stencil_texture);
		m_d3d_device->CreateDepthStencilView(m_depth_stencil_texture, NULL, &m_depth_stencil_view);
	}

	void GraphicsDevice::ClearShaderResources()
	{
		ID3D11ShaderResourceView *empty[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		m_immediate_context->PSSetShaderResources(0, 8, empty);
	}
}
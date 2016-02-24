#pragma once

namespace DX
{
    // 提供一个接口，以使拥有 DeviceResources 的应用程序在设备丢失或创建时收到通知。
	interface IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
        virtual void OnInitEngine() = 0;
	};

	// 控制所有 DirectX 设备资源。
	class DeviceResources
	{
	public:
		DeviceResources();
		void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
		void SetLogicalSize(Windows::Foundation::Size logicalSize);
		void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
		void SetDpi(float dpi);
		void SetCompositionScale(float compositionScaleX, float compositionScaleY);
		void ValidateDevice();
		void HandleDeviceLost();
		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
		void Trim();
		void Present();
        bool HasInit();

		// 设备访问器。
		Windows::Foundation::Size GetOutputSize() const					{ return m_outputSize; }
		Windows::Foundation::Size GetLogicalSize() const				{ return m_logicalSize; }

		// D3D 访问器。
		ID3D11Device2*			GetD3DDevice() const					{ return m_d3dDevice.Get(); }
		ID3D11DeviceContext2*	GetD3DDeviceContext() const				{ return m_d3dContext.Get(); }
		IDXGISwapChain1*		GetSwapChain() const					{ return m_swapChain.Get(); }
		D3D_FEATURE_LEVEL		GetDeviceFeatureLevel() const			{ return m_d3dFeatureLevel; }
		ID3D11RenderTargetView*	GetBackBufferRenderTargetView() const	{ return m_d3dRenderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const				{ return m_d3dDepthStencilView.Get(); }

		float GetCompositionScaleX() const {return m_compositionScaleX;}
		float GetCompositionScaleY() const {return m_compositionScaleY;}

	private:
		void CreateDeviceResources();
		void CreateWindowSizeDependentResources();
		DXGI_MODE_ROTATION ComputeDisplayRotation();
        void InitEngine();

		// Direct3D 对象。
		Microsoft::WRL::ComPtr<ID3D11Device2>			m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext2>	m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_swapChain;

		// Direct3D 渲染对象。3D 所必需的。
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;

		//对 XAML 面板的缓存引用。
		Windows::UI::Xaml::Controls::SwapChainPanel^    m_swapChainPanel;

		// 缓存的设备属性。
		D3D_FEATURE_LEVEL								m_d3dFeatureLevel;
		Windows::Foundation::Size						m_d3dRenderTargetSize;
		Windows::Foundation::Size						m_outputSize;
		Windows::Foundation::Size						m_logicalSize;
		Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
		Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
		float											m_dpi;
		float											m_compositionScaleX;
		float											m_compositionScaleY;

		// IDeviceNotify 可直接保留，因为它拥有 DeviceResources。
		IDeviceNotify* m_deviceNotify;
        bool m_init;
		DXGI_MODE_ROTATION m_displayRotation;
	};
}
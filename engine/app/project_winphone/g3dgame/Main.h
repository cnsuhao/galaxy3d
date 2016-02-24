#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"

// 在屏幕上呈现 3D 内容。
namespace g3dgame
{
	class Main : public DX::IDeviceNotify
	{
	public:
		Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Main();
        void Done();
		void CreateWindowSizeDependentResources();
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();
        virtual void OnInitEngine();

		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

	private:
		void Update();
		bool Render();

		// 缓存的设备资源指针。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// 渲染循环计时器。
		DX::StepTimer m_timer;
        bool m_done;
	};
}
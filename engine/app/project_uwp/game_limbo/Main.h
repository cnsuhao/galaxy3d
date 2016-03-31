#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"

// 在屏幕上呈现 Direct2D 和 3D 内容。
namespace game
{
	class Main : public DX::IDeviceNotify
	{
	public:
		Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();
		virtual void OnInitEngine();

		void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e);

	private:
		// 缓存的设备资源指针。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// 渲染循环计时器。
		DX::StepTimer m_timer;
	};
}
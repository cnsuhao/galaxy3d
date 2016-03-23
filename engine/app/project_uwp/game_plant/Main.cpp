#include "pch.h"
#include "Main.h"
#include "DirectXHelper.h"

using namespace game;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Concurrency;

#include "Screen.h"
#include "GraphicsDevice.h"
#include "World.h"
#include "Camera.h"
#include "Debug.h"
#include "../../src/SplashScreen.hpp"

using namespace Galaxy3D;

struct MouseEvent
{
	int type;
	float x;
	float y;
};

std::list<MouseEvent> g_event_queue;
std::shared_ptr<unsigned int> g_down_pointer_id;

extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

// 加载应用程序时加载并初始化应用程序资产。
Main::Main(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// 注册以在设备丢失或重新创建时收到通知
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: 将此替换为应用程序内容的初始化。

	// TODO: 如果需要默认的可变时间步长模式之外的其他模式，请更改计时器设置。
	// 例如，对于 60 FPS 固定时间步长更新逻辑，请调用:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

Main::~Main()
{
	// 取消注册设备通知
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

void Main::OnInitEngine()
{
    World::Init();

    GameObject::Create("")->AddComponent<SplashScreen>();
}

// 在窗口大小更改(例如，设备方向更改)时更新应用程序状态
void Main::CreateWindowSizeDependentResources() 
{
	// TODO: 将此替换为应用程序内容的与大小相关的初始化。
}

void Main::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ e)
{
	float x = e->CurrentPoint->Position.X * m_deviceResources->GetCompositionScaleX();
	float y = e->CurrentPoint->Position.Y * m_deviceResources->GetCompositionScaleY();

	Debug::Log("OnPointerPressed");

	if(!g_down_pointer_id)
	{
		g_down_pointer_id = std::make_shared<unsigned int>(e->CurrentPoint->PointerId);

		g_mouse_button_down[0] = true;
		g_mouse_position.x = (float) x;
		g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
		g_mouse_button_held[0] = true;
	}
}

void Main::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ e)
{
	float x = e->CurrentPoint->Position.X * m_deviceResources->GetCompositionScaleX();
	float y = e->CurrentPoint->Position.Y * m_deviceResources->GetCompositionScaleY();

	Debug::Log("OnPointerMoved");

	if(g_down_pointer_id && *g_down_pointer_id == e->CurrentPoint->PointerId)
	{
		g_mouse_position.x = (float) x;
		g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
	}
}

void Main::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ e)
{
	float x = e->CurrentPoint->Position.X * m_deviceResources->GetCompositionScaleX();
	float y = e->CurrentPoint->Position.Y * m_deviceResources->GetCompositionScaleY();

	Debug::Log("OnPointerReleased");

	if(g_down_pointer_id && *g_down_pointer_id == e->CurrentPoint->PointerId)
	{
		g_down_pointer_id.reset();

		g_mouse_button_up[0] = true;
		g_mouse_position.x = (float) x;
		g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
		g_mouse_button_held[0] = false;
	}
}

// 每帧更新一次应用程序状态。
void Main::Update() 
{
	if(!m_deviceResources->HasInit())
	{
		return;
	}

	// 更新场景对象。
	m_timer.Tick([&]()
	{
		// TODO: 将此替换为应用程序内容的更新函数。
		World::Update();
	});
}

// 根据当前应用程序状态呈现当前帧。
// 如果帧已呈现并且已准备好显示，则返回 true。
bool Main::Render() 
{
	bool rendered = false;

	// 在首次更新前，请勿尝试呈现任何内容。
	if(m_timer.GetFrameCount() == 0)
	{
		return rendered;
	}

	// 呈现场景对象。
	// TODO: 将此替换为应用程序内容的渲染函数。
	if(m_deviceResources->HasInit())
	{
		rendered = Camera::RenderAll();
	}

	return rendered;
}

// 通知呈现器，需要释放设备资源。
void Main::OnDeviceLost()
{
}

// 通知呈现器，现在可重新创建设备资源。
void Main::OnDeviceRestored()
{
	CreateWindowSizeDependentResources();
}

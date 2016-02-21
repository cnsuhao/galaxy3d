#include "pch.h"
#include "Main.h"
#include "DirectXHelper.h"

using namespace g3dgame;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

#include "Screen.h"
#include "GraphicsDevice.h"
#include "World.h"
#include "Camera.h"
#include "../../src/LauncherMerged.h"

using namespace Galaxy3D;

// 加载应用程序时加载并初始化应用程序资产。
Main::Main(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
    m_done(false)
{
	// 注册以在设备丢失或重新创建时收到通知
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: 将此替换为应用程序内容的初始化。
    Windows::UI::ViewManagement::StatusBar::GetForCurrentView()->HideAsync();

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
    GraphicsDevice::GetInstance()->Init(0);
    World::Init();

    GameObject::Create("")->AddComponent<LauncherMerged>();
}

void Main::Done()
{
    m_done = true;
    World::Done();
    GraphicsDevice::Done();
}

// 在窗口大小更改(例如，设备方向更改)时更新应用程序状态
void Main::CreateWindowSizeDependentResources()
{
	// TODO: 将此替换为应用程序内容的与大小相关的初始化。
}

void Main::StartRenderLoop()
{
	// 如果动画呈现循环已在运行，则勿启动其他线程。
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	//创建一个将在后台线程上运行的任务。
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// 计算更新的帧并且在每个场消隐期呈现一次。
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present();
			}
		}
	});

	// 在高优先级的专用后台线程上运行任务。
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void Main::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

// 每帧更新一次应用程序状态。
void Main::Update()
{
	// 更新场景对象。
	m_timer.Tick([&]()
	{
		// TODO: 将此替换为应用程序内容的更新函数。
        if(m_deviceResources->HasInit() && !m_done)
        {
            World::Update();
        }
	});
}

// 根据当前应用程序状态呈现当前帧。
// 如果帧已呈现并且已准备好显示，则返回 true。
bool Main::Render()
{
	// 在首次更新前，请勿尝试呈现任何内容。
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	// 呈现场景对象。
	// TODO: 将此替换为应用程序内容的渲染函数。
    if(m_deviceResources->HasInit() && !m_done)
    {
        Camera::RenderAll();
    }

	return true;
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

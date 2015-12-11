#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <cstdio>

#include "World.h"
#include "GraphicsDevice.h"
#include "Screen.h"
#include "Camera.h"
#include "Launcher.h"
#include "Debug.h"

#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "png.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "bullet.lib")
#pragma comment(lib, "galaxy3d.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Galaxy3D;

static const char g_title[] = "Galaxy3D Engine";
static const int g_screen_w = 1280;
static const int g_screen_h = 720;
HINSTANCE g_hinst;
HWND g_hwnd;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, int width, int height);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AllocConsole();
	SetConsoleTitle("Galaxy3D Console");
	FILE* fstdout = 0;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);

	if(FAILED(InitWindow(hInstance, nCmdShow, g_screen_w, g_screen_h)))
		return 0;

	Screen::SetSize(g_screen_w, g_screen_h);
	GraphicsDevice::GetInstance()->Init(g_hwnd);
	World::Init();

	GameObject::Create("launcher")->AddComponent<Launcher>();

	// Main message loop
	MSG msg = {0};
	while(WM_QUIT != msg.message)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
            World::Update();
            Camera::RenderAll();
		}
	}

	World::Done();

	if(fstdout != 0)
	{
		fclose(fstdout);
	}

	return (int) msg.wParam;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, int width, int height)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = g_title;
	wcex.hIconSm = 0;
	if(!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hinst = hInstance;
	RECT rc = {0, 0, width, height};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hwnd = CreateWindow(g_title, g_title, WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if(!g_hwnd)
		return E_FAIL;

	ShowWindow(g_hwnd, nCmdShow);

	return S_OK;
}

extern std::vector<Touch> g_input_touches;
extern std::list<Touch> g_input_touch_buffer;
bool g_input_down = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_LBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if(!g_input_down)
			{
				Touch t;
				t.deltaPosition = Vector2(0, 0);
				t.deltaTime = 0;
				t.fingerId = 0;
				t.phase = TouchPhase::Began;
				t.position = Vector2((float) x, (float) Screen::GetHeight() - y - 1);
				t.tapCount = 1;
				t.time = GTTime::GetRealTimeSinceStartup();

				if(!g_input_touches.empty())
				{
					g_input_touch_buffer.push_back(t);
				}
				else
				{
					g_input_touches.push_back(t);
				}

				g_input_down = true;
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if(g_input_down)
			{
				Touch t;
				t.deltaPosition = Vector2(0, 0);
				t.deltaTime = 0;
				t.fingerId = 0;
				t.phase = TouchPhase::Moved;
				t.position = Vector2((float) x, (float) Screen::GetHeight() - y - 1);
				t.tapCount = 1;
				t.time = GTTime::GetRealTimeSinceStartup();

				if(!g_input_touches.empty())
				{
					if(g_input_touch_buffer.empty())
					{
						g_input_touch_buffer.push_back(t);
					}
					else
					{
						if(g_input_touch_buffer.back().phase == TouchPhase::Moved)
						{
							g_input_touch_buffer.back() = t;
						}
						else
						{
							g_input_touch_buffer.push_back(t);
						}
					}
				}
				else
				{
					g_input_touches.push_back(t);
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if(g_input_down)
			{
				Touch t;
				t.deltaPosition = Vector2(0, 0);
				t.deltaTime = 0;
				t.fingerId = 0;
				t.phase = TouchPhase::Ended;
				t.position = Vector2((float) x, (float) Screen::GetHeight() - y - 1);
				t.tapCount = 1;
				t.time = GTTime::GetRealTimeSinceStartup();

				if(!g_input_touches.empty())
				{
					g_input_touch_buffer.push_back(t);
				}
				else
				{
					g_input_touches.push_back(t);
				}

				g_input_down = false;
			}
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

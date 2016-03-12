#include "Application.h"
#include "GTString.h"

#ifdef IOS
#import <Foundation/Foundation.h>
#endif

#ifdef WINPC
#include <Windows.h>
extern HWND g_hwnd;
#endif

#ifdef WINPHONE
#include <wrl/client.h>
#endif

namespace Galaxy3D
{
	std::string Application::m_data_path;

	std::string Application::GetSavePath()
	{
		static std::string s_save_path;

#ifdef WINPC
		if(s_save_path.empty())
		{
			s_save_path = GetDataPath();
		}
#endif

#ifdef WINPHONE
		if(s_save_path.empty())
		{
			char buffer[MAX_PATH];
			Platform::String^ path_name = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
			const wchar_t *wpath = path_name->Data();
			WideCharToMultiByte(CP_ACP, 0, wpath, -1, buffer, MAX_PATH, NULL, NULL);
			GTString path = GTString(buffer).Replace("\\", "/");
			s_save_path = path.str;
		}
#endif

#ifdef ANDROID
		if(s_save_path.empty())
		{
			s_save_path = GetDataPath();
		}
#endif

		return s_save_path;
	}

    std::string Application::GetDataPath()
    {
#ifdef WINPC
		if(m_data_path.empty())
		{
			char buffer[MAX_PATH];
			GetModuleFileNameA(0, buffer, MAX_PATH);
			GTString path = GTString(buffer).Replace("\\", "/");
			m_data_path = path.str;
			m_data_path = m_data_path.substr(0, path.str.rfind('/'));
		}
#endif

#ifdef WINPHONE
		if(m_data_path.empty())
		{
			char buffer[MAX_PATH];
			Platform::String^ path_name = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
			const wchar_t *wpath = path_name->Data();
			WideCharToMultiByte(CP_ACP, 0, wpath, -1, buffer, MAX_PATH, NULL, NULL);
			GTString path = GTString(buffer).Replace("\\", "/");
			m_data_path = path.str;
		}
#endif

#ifdef IOS
		if(m_data_path.empty())
		{
			m_data_path = [[[NSBundle mainBundle] resourcePath] UTF8String];
		}
#endif

#ifdef ANDROID

#endif

		return m_data_path;
    }

    void Application::Quit()
    {
#ifdef WINPC
        SendMessage(g_hwnd, WM_CLOSE, 0, 0);
#endif
    }
}
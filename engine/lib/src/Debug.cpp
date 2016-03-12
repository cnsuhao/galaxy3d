#include "Debug.h"

#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <map>
#include <list>

#define LOG_TO_FILE 0

#ifdef WINPHONE
#include <windows.h>
#endif

#ifdef ANDROID
#include <android/log.h>
#endif

#ifdef IOS
#import <Foundation/Foundation.h>
#endif

namespace Galaxy3D
{
	void Debug::Log(const char *format, ...)
	{
		const static int BUFFER_SIZE = 1024;

		va_list plist;
		va_start(plist, format);
	
		char buffer[BUFFER_SIZE];
		vsnprintf(buffer, BUFFER_SIZE, format, plist);

		va_end(plist);

#if LOG_TO_FILE
		std::ofstream of("DebugLog.txt", std::ios::app);
		of << buffer << std::endl;
		of.close();
#else
	#ifdef ANDROID
		__android_log_print(ANDROID_LOG_ERROR, "Galaxy3D", "%s", buffer);
	#endif

	#ifdef IOS
		NSLog(@"%s", buffer);
	#endif
    
	#ifdef WIN32
#ifdef WINPHONE
#ifdef _DEBUG
		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
#endif
#else
		std::cout << buffer << std::endl;
#endif
	#endif
#endif
	}
}

#include "GTTime.h"

#ifdef WINPHONE
#include <wrl.h>
static long long g_frequency = -1L;
#endif

#ifdef WIN32
#include <windows.h>
#endif

#if defined(IOS) || defined(ANDROID)
#include <sys/time.h>
#endif

namespace Galaxy3D
{
	long long GTTime::m_time_startup;
	long long GTTime::m_time_update;
	float GTTime::m_time_delta = 0;
	float GTTime::m_time_record = -1;
	int GTTime::m_frame_count = 0;
	int GTTime::m_frame_record;
	int GTTime::m_fps;
	float GTTime::m_render_time;
	float GTTime::m_update_time;

	long long GTTime::GetTimeMS()
	{
		long long t;

#ifdef WINPC
		t = timeGetTime();
#endif

#ifdef WINPHONE
		if (g_frequency < 0)
		{
			QueryPerformanceFrequency((LARGE_INTEGER *) &g_frequency);
		}
		
		LARGE_INTEGER c;
		QueryPerformanceCounter(&c);
		t = (long long) (((double) c.QuadPart / (double) g_frequency) * 1000);
#endif
    
#if IOS || ANDROID
		struct timeval tv;
		gettimeofday(&tv, NULL);
		t = tv.tv_sec;
		t *= 1000;
		t += tv.tv_usec/1000;
#endif

		return t;
	}

	float GTTime::GetRealTimeSinceStartup()
	{
		long long time = GetTimeMS() - m_time_startup;

		return time / 1000.0f;
	}
}
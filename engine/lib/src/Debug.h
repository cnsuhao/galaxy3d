#ifndef __Debug_h__
#define __Debug_h__

#include <string>

namespace Galaxy3D
{
	class Debug
	{
	public:
		static void Log(const char *format, ...);
	};
}

#endif
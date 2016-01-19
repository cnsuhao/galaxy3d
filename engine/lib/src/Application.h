#ifndef __Application_h__
#define __Application_h__

#include <string>

namespace Galaxy3D
{
	class Application
    {
    public:
        static std::string GetDataPath();
        static void Quit();
    };
}

#endif
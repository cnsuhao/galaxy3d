#ifndef __Application_h__
#define __Application_h__

#include <string>

namespace Galaxy3D
{
	class Application
    {
    public:
        static std::string GetDataPath();
		static void SetDataPath(const std::string &path) {m_data_path = path;}
		static std::string GetSavePath();
        static void Quit();

	private:
		static std::string m_data_path;
    };
}

#endif
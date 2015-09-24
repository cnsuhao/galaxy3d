#ifndef __GTFile_h__
#define __GTFile_h__

#include <vector>
#include <string>

namespace Galaxy3D
{
	class GTFile
	{
	public:
		static bool Exist(const std::string &path);
        // 用完之后手动free释放
		static void *ReadAllBytes(const std::string &path, int *size);
		static void ReadAllText(const std::string &path, std::string &str);
		static void WriteAllBytes(const std::string &path, void *data, int size);
	};
}

#endif
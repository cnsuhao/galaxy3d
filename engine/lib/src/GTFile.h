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
		static std::vector<char> ReadAllBytes(const std::string &path);
		static std::string ReadAllText(const std::string &path);
		static void WriteAllBytes(const std::string &path, void *data, int size);
	};
}

#endif
#ifndef __GTFile_h__
#define __GTFile_h__

#include <vector>
#include <string>

#define BUFFER_READ(dest, p, size) if(size > 0){memcpy(&dest, p, size);p += size;}

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
		static void CreateFolder(const std::string &path);
		static void Unzip(const std::string &zip_path, const std::string &source, const std::string &dest, bool directory);
	};
}

#endif
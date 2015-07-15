#ifndef __MirImage_h__
#define __MirImage_h__

#include "Texture2D.h"
#include <unordered_map>

#ifdef LoadImage
#undef LoadImage
#endif

using namespace Galaxy3D;

#pragma pack(2)
struct WZX
{
	static const int STRUCT_SIZE = 48;
	static const int FLAG_SIZE = 44;
	
	char flag[FLAG_SIZE];
	int image_count;
};

struct MirImageFormat
{
	enum Enum
	{
		None = 0,
		Index8 = 0x103,
		RGB565 = 0x105,
	};
};

struct MirImageData
{
	static const int INFO_SIZE = 16;

    short format;
    short unknown;
    short w;
    short h;
    short x;
    short y;
    int zip_size;
	std::vector<char> bmp;
};
#pragma pack()

class MirImage
{
public:
	static std::vector<std::shared_ptr<Texture2D>> LoadImages(const std::string &name, const std::vector<int> &indices);

private:
	static std::unordered_map<std::string, std::unordered_map<int, std::weak_ptr<Texture2D>>> m_cache;
	
	static std::shared_ptr<Texture2D> LoadImage(FILE *f, int offset);
};

#endif
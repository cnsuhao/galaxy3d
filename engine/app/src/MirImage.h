#ifndef __MirImage_h__
#define __MirImage_h__

#include "Texture2D.h"
#include <unordered_map>

#ifdef LoadImage
#undef LoadImage
#endif

#define MIR_PIXEL_TO_UNIT 0.01f

using namespace Galaxy3D;

#pragma pack(2)
struct WZX
{
	static const int STRUCT_SIZE = 48;
	static const int FLAG_SIZE = 44;
	
	char flag[FLAG_SIZE];
	int image_count;
};

struct WZL
{
	static const int INFO_SIZE = 56;
	static const int FLAG_SIZE = 44;
	
	char flag[FLAG_SIZE];
	int image_count;
	int color_count;
	int palette_size;

	std::vector<char> palette;
};

struct MirImageFormat
{
	enum Enum
	{
		None = 0,
		Index8 = 0x0103,
		RGB565 = 0x0105,
	};
};

struct MirImageData
{
	static const int INFO_SIZE = 16;

    short format;
	short unknow;
    short w;
    short h;
    short x;
    short y;
    int zip_size;
	std::vector<char> bmp;
};
#pragma pack()

struct MirImage
{
public:
	MirImageData data;
	std::shared_ptr<Texture2D> texture;

	static std::vector<std::shared_ptr<MirImage>> LoadImages(const std::string &name, const std::vector<int> &indices);
	static std::shared_ptr<MirImage> LoadImage(const std::string &name, int index);
	static std::shared_ptr<Texture2D> GetColorTable();

private:
	static std::unordered_map<std::string, std::unordered_map<int, std::weak_ptr<MirImage>>> m_cache;
	static std::shared_ptr<Texture2D> m_color_table;

	static std::shared_ptr<MirImage> LoadImage(FILE *f, int offset, const std::string &name, int index);
};

#endif
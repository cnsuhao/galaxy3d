#ifndef __Image_h__
#define __Image_h__

#include <string>

namespace Galaxy3D
{
	class Texture2D;

	class Image
	{
	public:
		//return pixels from malloc
		static char *LoadJpg(char *data, int size, int *w, int *h, int *bitsPerPixel);
		static char *LoadPng(char *data, int size, int *w, int *h, int *bitsPerPixel);
		static void EncodeToPNG(Texture2D *tex, int bits_per_pixel, const std::string &file);
	};
}

#endif
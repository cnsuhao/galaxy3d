#ifndef __Image_h__
#define __Image_h__

namespace Galaxy3D
{
	class Image
	{
	public:
		//return pixels from malloc
		static char *LoadJpg(char *data, int size, int *w, int *h, int *bitsPerPixel);
		static char *LoadPng(char *data, int size, int *w, int *h, int *bitsPerPixel);
	};
}

#endif
#include "Image.h"
#include "Texture2D.h"
#include "GTFile.h"
#include <stdlib.h>
#include <stdio.h>

extern "C" {
#include "../depend/jpeg-8d/jpeglib.h"
#include "../depend/lpng1512/png.h"
#include "../depend/lpng1512/pngstruct.h"
}

namespace Galaxy3D
{
	struct PNGData
	{
		char *buffer;
		int size;
	};

	char *Image::LoadJpg(void *data, int size, int *w, int *h, int *bitsPerPixel)
	{
		char *pixels = 0;

		//use lib jpeg
		jpeg_decompress_struct cinfo;
		jpeg_error_mgr jerr;
		JSAMPARRAY buffer;		/* Output row buffer */
		int row_stride;		/* physical row width in output buffer */

		cinfo.err = jpeg_std_error(&jerr);

		jpeg_create_decompress(&cinfo);
		jpeg_mem_src(&cinfo, (unsigned char *) data, size);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
		row_stride = cinfo.output_width * cinfo.output_components;
		/* Make a one-row-high sample array that will go away when done with image */
		buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

		*w = cinfo.output_width;
		*h = cinfo.output_height;
		*bitsPerPixel = cinfo.output_components * 8;

		pixels = (char *) malloc((*w)*(*h)*cinfo.output_components);

		unsigned char *pPixel = (unsigned char *) &pixels[0];

		while(cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);

			unsigned char *pixel_data = buffer[0];

			for(int j=0; j<*w; j++)
			{
				memcpy(pPixel, pixel_data, cinfo.output_components);

				pPixel += cinfo.output_components;
				pixel_data += cinfo.output_components;
			}
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	
		return pixels;
	}

	static void user_png_read(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		memcpy(data, png_ptr->io_ptr, length);
		png_ptr->io_ptr = (char *) png_ptr->io_ptr + length;
	}

	static void user_png_write(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		PNGData *png = (PNGData *) png_get_io_ptr(png_ptr);

		if(png->buffer == 0)
		{
			png->buffer = (char *) malloc(length);
		}
		else
		{
			png->buffer = (char *) realloc(png->buffer, png->size + length);
		}

		memcpy(&png->buffer[png->size], data, length);
		png->size += length;
	}

	static void user_png_flush(png_structp png_ptr)
	{
	}

	char *Image::LoadPng(void *data, int size, int *w, int *h, int *bitsPerPixel)
	{
		char *pixels = 0;
		
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));
	
		png_set_read_fn(png_ptr, data, user_png_read);
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

		*w = png_get_image_width(png_ptr, info_ptr);
		*h = png_get_image_height(png_ptr, info_ptr);

		int color_type = png_get_color_type(png_ptr, info_ptr);
		if(color_type == PNG_COLOR_TYPE_RGBA)
		{
			*bitsPerPixel = png_get_bit_depth(png_ptr, info_ptr) * 4;

			png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

			pixels = (char *) malloc((*h) * (*w) * 4);

			unsigned char *pPixel = (unsigned char *) &pixels[0];

			for(int i=0; i<*h; i++)
			{
				memcpy(pPixel, row_pointers[i], (*w)*4);
				pPixel += (*w)*4;
			}
		}
		else if(color_type == PNG_COLOR_TYPE_RGB)
		{
			*bitsPerPixel = png_get_bit_depth(png_ptr, info_ptr) * 3;

			png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

			pixels = (char *) malloc((*h) * (*w) * 3);

			unsigned char *pPixel = (unsigned char *) &pixels[0];

			for(int i=0; i<*h; i++)
			{
				memcpy(pPixel, row_pointers[i], (*w)*3);
				pPixel += (*w)*3;
			}
		}
		else if(color_type == PNG_COLOR_TYPE_GRAY)
		{
			*bitsPerPixel = png_get_bit_depth(png_ptr, info_ptr) * 1;

			png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

			pixels = (char *) malloc((*h) * (*w));

			unsigned char *pPixel = (unsigned char *) &pixels[0];

			for(int i=0; i<*h; i++)
			{
				memcpy(pPixel, row_pointers[i], (*w)*1);
				pPixel += (*w)*1;
			}
		}

		png_destroy_read_struct(&png_ptr, &info_ptr, 0);

		return pixels;
	}

	void Image::EncodeToPNG(Texture2D *tex, int bits_per_pixel, const std::string &file)
	{
		int color_type = -1;
		switch(bits_per_pixel)
		{
		case 32:
			color_type = PNG_COLOR_TYPE_RGBA;
			break;
		case 24:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case 8:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
		default:
			return;
		}

		char *data = 0;
		int width = tex->GetWidth();
		int height = tex->GetHeight();
		const char *colors = tex->GetPixels();
		if(colors == nullptr)
		{
			return;
		}

		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));

		PNGData png_data;
		png_data.buffer = 0;
		png_data.size = 0;

		png_set_write_fn(png_ptr, &png_data, user_png_write, user_png_flush);
		
		png_set_IHDR(png_ptr, info_ptr, width, height,
			8, color_type, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_write_info(png_ptr, info_ptr);

		png_bytepp row_pointers = new png_bytep[height];
		for(int i=0; i<height; i++)
		{
			row_pointers[i] = (png_bytep) &colors[i * width * bits_per_pixel / 8];
		}
		png_write_image(png_ptr, row_pointers);
		delete [] row_pointers;

		png_write_end(png_ptr, 0);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		data = png_data.buffer;
		int size = png_data.size;

		GTFile::WriteAllBytes(file, data, size);

		free(data);
	}
}
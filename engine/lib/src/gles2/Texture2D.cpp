#include "Texture2D.h"
#include "Image.h"
#include "GTFile.h"
#include "Debug.h"

namespace Galaxy3D
{
    std::shared_ptr<Texture2D> Texture2D::m_default_texture;

	static const unsigned char JPG_HEAD[] = {0xff, 0xd8, 0xff};
	static const unsigned char PNG_HEAD[] = {0x89, 0x50, 0x4e, 0x47};

	const int Texture::PIXEL_BITS_SIZE[20] =
	{
		8,
		16,
		24,
		32,
		32,
		16,
		-1,
		-1,
		16,
		2,
		2,
		4,
		4,
		4,
		4,
		8,
		32,
		-1,
		-1,
		-1,
	};

	struct GLTextureFormat
    {
        GLenum internal_format;
        GLenum format;
        GLenum type;
    };

	static const GLTextureFormat TEXTURE_FORMATS[20] =
	{
		{GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE},
        {GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
        {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
        {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
        {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
        {GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},
        {0, 0, 0},
        {0, 0, 0},
        {GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
#ifdef IOS
        {GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0xffffffff, 0xffffffff},
        {GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0xffffffff, 0xffffffff},
        {GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0xffffffff, 0xffffffff},
        {GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0xffffffff, 0xffffffff},
#else
		{0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
		{0, 0, 0},
#endif
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
	};

	static const GLenum FILTER_MODES[3] =
	{
		GL_NEAREST,
		GL_LINEAR,
		GL_LINEAR,
	};
	const GLenum *Texture::FILTER_MODES = Galaxy3D::FILTER_MODES;
	
	static const GLenum FILTER_MODES_MIP[3] =
	{
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR,
	};
    
	static const GLenum ADDRESS_MODES[2] =
	{
		GL_REPEAT,
		GL_CLAMP_TO_EDGE,
	};
	const GLenum *Texture::ADDRESS_MODES = Galaxy3D::ADDRESS_MODES;

	Texture2D::~Texture2D()
	{
		if(m_texture != 0)
		{
			glDeleteTextures(1, &m_texture);
			m_texture = 0;
		}

		if(m_colors != NULL)
		{
			free(m_colors);
			m_colors = NULL;
			m_color_buffer_size = 0;
		}
	}

	std::shared_ptr<Texture2D> Texture2D::GetDefaultTexture()
    {
        if(!m_default_texture)
        {
            m_default_texture = Texture2D::Create(1, 1, TextureFormat::RGBA32, FilterMode::Point, TextureWrapMode::Clamp);
            unsigned char buffer[4] = {255, 255, 255, 255};
            m_default_texture->SetPixels((char *) buffer);
            m_default_texture->Apply();
        }

        return m_default_texture;
    }

	std::shared_ptr<Texture2D> Texture2D::Create(int w, int h, TextureFormat::Enum format, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode)
	{
		return std::shared_ptr<Texture2D>(new Texture2D(w, h, format, filter_mode, wrap_mode));
	}

	std::shared_ptr<Texture2D> Texture2D::CreateWithData(void *data, int size, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode, bool mipmap)
	{
		std::shared_ptr<Texture2D> tex;

		int w, h;
		int bpp = 0;
		char *pixels = 0;

		if(memcmp(data, JPG_HEAD, 3) == 0)
		{
			pixels = Image::LoadJpg(data, size, &w, &h, &bpp);
		}
		else if(memcmp(data, PNG_HEAD, 4) == 0)
		{
			pixels = Image::LoadPng(data, size, &w, &h, &bpp);
		}

		TextureFormat::Enum format;

		if(bpp == 32)
		{
			format = TextureFormat::RGBA32;
		}
		else if(bpp == 24)
		{
			format = TextureFormat::RGB24;
		}
		else if(bpp == 8)
		{
			format = TextureFormat::Alpha8;
		}
		else
		{
			if(pixels != 0)
			{
				free(pixels);
				pixels = 0;
			}
		}

		if(pixels != 0)
		{
			tex = Create(w, h, format, filter_mode, wrap_mode);
            tex->m_mipmap = mipmap;
			tex->SetPixels(pixels);
			tex->Apply();

			free(pixels);
			pixels = 0;
		}

		return tex;
	}

	std::shared_ptr<Texture2D> Texture2D::LoadFromFile(const std::string &file, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode, bool mipmap)
	{
        std::shared_ptr<Texture2D> tex;

        auto find = FindCachedObject(file);
        if(find)
        {
            tex = std::dynamic_pointer_cast<Texture2D, Object>(find);
        }
        else if(GTFile::Exist(file))
        {
            int file_size;
            char *bytes = (char *) GTFile::ReadAllBytes(file, &file_size);
            if(bytes != NULL)
            {
                tex = CreateWithData(bytes, file_size, filter_mode, wrap_mode, mipmap);
                free(bytes);

                tex->SetName(file);
                CacheObject(file, tex);
            }
        }
		else
		{
			Debug::Log("file %s not exist", file.c_str());
		}

		return tex;
	}

	void Texture2D::SetPixels(const char *colors)
	{
		if(PIXEL_BITS_SIZE[m_format] > 0)
		{
			int size = m_width * m_height * PIXEL_BITS_SIZE[m_format] / 8;

			if(m_colors == 0)
			{
				m_colors = (char *) calloc(1, size);
			}
			else
			{
				if(m_color_buffer_size != size)
				{
					m_colors = (char *) realloc(m_colors, size);
				}
			}

			m_color_buffer_size = size;

			memcpy(m_colors, colors, size);
		}
	}

	void Texture2D::SetPixels(int x, int y, int w, int h, const char *colors)
	{
		if(PIXEL_BITS_SIZE[m_format] > 0)
		{
			int pixel_size = PIXEL_BITS_SIZE[m_format] / 8;
			int size = m_width * m_height * pixel_size;

			if(m_colors == 0)
			{
				m_colors = (char *) calloc(1, size);
			}
			else
			{
				if(m_color_buffer_size != size)
				{
					m_colors = (char *) realloc(m_colors, size);
				}
			}

			m_color_buffer_size = size;

			for(int i=0; i<h; i++)
			{
				if(	x >= 0 &&
					y >= 0 &&
					x + w <= m_width &&
					y + h <= m_height)
				{
					memcpy(&m_colors[((y + i) * m_width + x) * pixel_size], &colors[i * w * pixel_size], w * pixel_size);
				}
			}
		}
	}

	void Texture2D::Apply()
	{
		bool mipmap = m_mipmap;

		if(m_texture == 0)
		{
			glGenTextures(1, &m_texture);
		}
		
        glBindTexture(GL_TEXTURE_2D, m_texture);

		if(mipmap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FILTER_MODES_MIP[m_filter_mode]);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FILTER_MODES[m_filter_mode]);
		}
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FILTER_MODES[m_filter_mode]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ADDRESS_MODES[m_wrap_mode]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ADDRESS_MODES[m_wrap_mode]);

		glTexImage2D(GL_TEXTURE_2D, 0, TEXTURE_FORMATS[m_format].internal_format, m_width, m_height, 0, TEXTURE_FORMATS[m_format].format, TEXTURE_FORMATS[m_format].type, m_colors);
		
		if(mipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

        glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::EncodeToPNG(const std::string &file)
	{
		Image::EncodeToPNG(this, PIXEL_BITS_SIZE[m_format], file);
	}
}
#include "Cubemap.h"
#include "Texture2D.h"
#include "Mathf.h"

namespace Galaxy3D
{
	std::shared_ptr<Cubemap> Cubemap::LoadFromFile(const std::vector<std::string> &files, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode, bool mipmap, bool mip_gen, int mip_count)
	{
		std::vector<std::shared_ptr<Texture2D>> texs;
        for(size_t i=0; i<files.size(); i++)
        {
            auto tex = Texture2D::LoadFromFile(files[i], filter_mode, wrap_mode, false);
            texs.push_back(tex);
        }

        auto map = std::shared_ptr<Cubemap>(new Cubemap(texs[0]->GetWidth(), filter_mode, wrap_mode));
		if(mipmap && !mip_gen)
		{
			map->m_colors.resize(mip_count * 6, NULL);
		}
		else
		{
			map->m_colors.resize(6, NULL);
		}

        for(size_t i=0; i<files.size(); i++)
        {
            auto pixels = texs[i]->GetPixels();
            map->SetPixels(pixels, texs[i]->GetColorBufferSize(), i);
        }
        map->m_format = texs[0]->GetFormat();
		map->m_mipmap = mipmap;
		map->m_mip_gen = mip_gen;
		map->m_mip_count = mip_count;
        map->Apply();

        return map;
	}

	Cubemap::~Cubemap()
	{
		if(m_texture != 0)
		{
			glDeleteTextures(1, &m_texture);
			m_texture = 0;
		}

		for(size_t i=0; i<m_colors.size(); i++)
		{
			if(m_colors[i] != NULL)
			{
				free(m_colors[i]);
				m_colors[i] = NULL;
			}
		}
		m_colors.clear();
	}

	void Cubemap::SetPixels(const char *colors, int size, int index)
	{
		if(index < (int) m_colors.size())
		{
			if(m_colors[index] == 0)
			{
				m_colors[index] = (char *) malloc(size);
			}
			else
			{
				m_colors[index] = (char *) realloc(m_colors[index], size);
			}

			memcpy(m_colors[index], colors, size);
		}
	}

	void Cubemap::Apply()
	{
		bool mipmap = m_mipmap;
		int mip_count;

		if(mipmap)
		{
			if(m_mip_gen)
			{
				mip_count = 0;
			}
			else
			{
				mip_count = m_mip_count;
			}
		}
		else
		{
			mip_count = 1;
		}

		int tex_count;
		if(mipmap && !m_mip_gen)
		{
			tex_count = mip_count * 6;
		}
		else
		{
			tex_count = 6;
		}

		if(m_texture == 0)
		{
			glGenTextures(1, &m_texture);
		}
		
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);

		if(mipmap)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FILTER_MODES_MIP[m_filter_mode]);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FILTER_MODES[m_filter_mode]);
		}
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, FILTER_MODES[m_filter_mode]);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, ADDRESS_MODES[m_wrap_mode]);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, ADDRESS_MODES[m_wrap_mode]);

		for(int i=0; i<tex_count; i++)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i % 6;
			int mip_level = i / 6;
			int mip_div = (int) pow(2, i / 6);
			int w = m_width / mip_div;
			int h = m_height / mip_div;

			glTexImage2D(face, mip_level,
				Texture::TEXTURE_FORMATS[m_format].internal_format,
				w, h, 0,
				Texture::TEXTURE_FORMATS[m_format].format,
				Texture::TEXTURE_FORMATS[m_format].type,
				m_colors[i]);
		}

		if(mipmap && m_mip_gen)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}
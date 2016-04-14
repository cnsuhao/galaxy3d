#ifndef __Cubemap_h__
#define __Cubemap_h__

#include "Texture.h"
#include "TextureFormat.h"

namespace Galaxy3D
{
    class Cubemap : public Texture
    {
    public:
        static std::shared_ptr<Cubemap> LoadFromFile(const std::vector<std::string> &files, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp, bool mipmap = false, bool mip_gen = true, int mip_count = 0);
        virtual ~Cubemap();
        GLuint GetTexture() const {return m_texture;}

    private:
        std::vector<char *> m_colors;
        TextureFormat::Enum m_format;
        GLuint m_texture;
		bool m_mipmap;
		bool m_mip_gen;
		int m_mip_count;

        Cubemap(int size, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode):
            Texture(size, size, filter_mode, wrap_mode),
            m_texture(0),
			m_mipmap(false),
			m_mip_gen(true),
			m_mip_count(0)
        {
        }
        void SetPixels(const char *colors, int size, int index);
        void Apply();
    };
}

#endif
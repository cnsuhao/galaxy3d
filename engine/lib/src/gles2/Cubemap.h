#ifndef __Cubemap_h__
#define __Cubemap_h__

#include "Texture.h"
#include "TextureFormat.h"

namespace Galaxy3D
{
    class Cubemap : public Texture
    {
    public:
        static std::shared_ptr<Cubemap> LoadFromFile(const std::vector<std::string> &files, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
        virtual ~Cubemap()
        {
        }
        GLuint GetTexture() const {return m_texture;}

    private:
        char *m_colors[6];
        TextureFormat::Enum m_format;
        GLuint m_texture;

        Cubemap(int size, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode):
            Texture(size, size, filter_mode, wrap_mode),
            m_texture(0)
        {
            memset(m_colors, 0, sizeof(m_colors));
        }
        void SetPixels(const char *colors, int size, int index);
        void Apply();
    };
}

#endif
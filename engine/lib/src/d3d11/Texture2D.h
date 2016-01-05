#ifndef __Texture2D_h__
#define __Texture2D_h__

#include "Texture.h"
#include "TextureFormat.h"
#include <d3d11.h>
#include <vector>
#include <unordered_map>

namespace Galaxy3D
{
	class Texture2D : public Texture
	{
	public:
		//support jpg 8 24, png 8 24 32
		static std::shared_ptr<Texture2D> Create(int w, int h, TextureFormat::Enum format, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
		static std::shared_ptr<Texture2D> CreateWithData(void *data, int size, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp, bool mipmap = false);
		//cached by file name
        static std::shared_ptr<Texture2D> LoadFromFile(const std::string &file, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp, bool mipmap = false);
		static std::shared_ptr<Texture2D> GetDefaultTexture();
        virtual ~Texture2D();
		void SetPixels(const char *colors);
		void SetPixels(int x, int y, int w, int h, const char *colors);
		void Apply();
		const char *GetPixels() const {return m_colors;}
        int GetColorBufferSize() const {return m_color_buffer_size;}
        TextureFormat::Enum GetFormat() const {return m_format;}
		void EncodeToPNG(const std::string &file);
		ID3D11ShaderResourceView *GetTexture() const {return m_texture_res;}
		ID3D11SamplerState *GetSampler() const {return m_sampler;}

	private:
        static std::shared_ptr<Texture2D> m_default_texture;
		char *m_colors;
		int m_color_buffer_size;
		TextureFormat::Enum m_format;
		ID3D11Texture2D *m_texture;
		ID3D11ShaderResourceView *m_texture_res;
		ID3D11SamplerState *m_sampler;
        bool m_mipmap;

		Texture2D(int w, int h, TextureFormat::Enum format, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode):
			Texture(w, h, filter_mode, wrap_mode),
			m_colors(NULL),
			m_color_buffer_size(0),
			m_format(format),
			m_texture(NULL),
			m_texture_res(NULL),
			m_sampler(NULL),
            m_mipmap(false)
		{}
	};
}

#endif
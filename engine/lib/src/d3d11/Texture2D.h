#ifndef __Texture2D_h__
#define __Texture2D_h__

#include "Texture.h"
#include "TextureFormat.h"
#include <d3d11.h>
#include <vector>

namespace Galaxy3D
{
	class Texture2D : public Texture
	{
	public:
		//support jpg 8 24, png 8 24 32
		static std::shared_ptr<Texture2D> Create(int w, int h, TextureFormat::Enum format, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
		static std::shared_ptr<Texture2D> LoadImageData(char *data, int size, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
		static std::shared_ptr<Texture2D> LoadImageFile(const std::string &file, FilterMode::Enum filter_mode = FilterMode::Bilinear, TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp);
		virtual ~Texture2D();
		void SetPixels(const char *colors);
		void Apply();
		ID3D11ShaderResourceView *GetTexture() const {return m_texture;}
		ID3D11SamplerState *GetSampler() const {return m_sampler;}

	private:
		char *m_colors;
		TextureFormat::Enum m_format;
		ID3D11ShaderResourceView *m_texture;
		ID3D11SamplerState *m_sampler;

		Texture2D(int w, int h, TextureFormat::Enum format, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode):
			Texture(w, h, filter_mode, wrap_mode),
			m_colors(nullptr),
			m_format(format),
			m_texture(nullptr),
			m_sampler(nullptr)
		{}
	};
}

#endif
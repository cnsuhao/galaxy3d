#include "Texture2D.h"
#include "GraphicsDevice.h"
#include "Image.h"
#include "GTFile.h"

namespace Galaxy3D
{
	static const unsigned char JPG_HEAD[] = {0xff, 0xd8, 0xff};
	static const unsigned char PNG_HEAD[] = {0x89, 0x50, 0x4e, 0x47};

	static const int PIXEL_BITS_SIZE[20] =
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

	static const DXGI_FORMAT TEXTURE_FORMATS[] =
	{
		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_B4G4R4A4_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_B5G6R5_UNORM,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_B4G4R4A4_UNORM,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
	};

	static const D3D11_FILTER FILTER_MODES[3] =
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
        D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC,
	};

	static const D3D11_TEXTURE_ADDRESS_MODE ADDRESS_MODES[2] =
	{
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
	};

	Texture2D::~Texture2D()
	{
		SAFE_RELEASE(m_texture);
		SAFE_RELEASE(m_texture_res);
		SAFE_RELEASE(m_sampler);

		if(m_colors != NULL)
		{
			free(m_colors);
			m_colors = NULL;
			m_color_buffer_size = 0;
		}
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
		auto device = GraphicsDevice::GetInstance()->GetDevice();
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		bool mipmap = m_mipmap;

		D3D11_TEXTURE2D_DESC desc;
        desc.Width = static_cast<UINT>(m_width);
        desc.Height = static_cast<UINT>(m_height);
        desc.MipLevels = mipmap ? 0 : 1;
        desc.ArraySize = 1;
        desc.Format = TEXTURE_FORMATS[m_format];
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = mipmap ? (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) : D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = mipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		D3D11_SUBRESOURCE_DATA init_data;
		ZeroMemory(&init_data, sizeof(init_data));
		char *buffer = 0;
		int bpp = PIXEL_BITS_SIZE[m_format];
		if(m_colors != 0 && bpp > 0)
		{
			int bytes = bpp / 8;
			if(m_format == TextureFormat::RGB24)
			{
				bytes = 4;
				buffer = new char[m_width * m_height * 4];
				for(int i=0; i<m_height; i++)
				{
					for(int j=0; j<m_width; j++)
					{
						buffer[i * m_width * 4 + j * 4 + 0] = m_colors[i * m_width * 3 + j * 3 + 0];
						buffer[i * m_width * 4 + j * 4 + 1] = m_colors[i * m_width * 3 + j * 3 + 1];
						buffer[i * m_width * 4 + j * 4 + 2] = m_colors[i * m_width * 3 + j * 3 + 2];
						buffer[i * m_width * 4 + j * 4 + 3] = (char) 0xff;
					}
				}

				init_data.pSysMem = buffer;
			}
			else
			{
				init_data.pSysMem = m_colors;
			}

			init_data.SysMemPitch = m_width * bytes;
		}

		if(m_texture == NULL)
		{
			HRESULT hr = device->CreateTexture2D(&desc, (mipmap || init_data.pSysMem == 0) ? 0 : &init_data, &m_texture);

			if(mipmap)
			{
				context->UpdateSubresource(m_texture, 0, 0, init_data.pSysMem, init_data.SysMemPitch, 0);
			}
		}
		else
		{
			context->UpdateSubresource(m_texture, 0, 0, init_data.pSysMem, init_data.SysMemPitch, 0);
		}
		
		if(buffer != 0)
		{
			delete [] buffer;
		}

		if(m_texture_res == NULL)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
			memset(&srvd, 0, sizeof(srvd));
			srvd.Format = TEXTURE_FORMATS[m_format];
			srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvd.Texture2D.MipLevels = -1;

			device->CreateShaderResourceView(m_texture, &srvd, &m_texture_res);

			if(mipmap)
			{
				context->GenerateMips(m_texture_res);
			}
		}

		if(m_sampler == NULL)
		{
			D3D11_SAMPLER_DESC sampDesc;
			ZeroMemory(&sampDesc, sizeof(sampDesc));
			sampDesc.Filter = FILTER_MODES[m_filter_mode];
			sampDesc.AddressU = ADDRESS_MODES[m_wrap_mode];
			sampDesc.AddressV = ADDRESS_MODES[m_wrap_mode];
			sampDesc.AddressW = ADDRESS_MODES[m_wrap_mode];
			sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampDesc.MinLOD = 0;
			sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
            sampDesc.MipLODBias = 0;
			device->CreateSamplerState(&sampDesc, &m_sampler);
		}
	}

	void Texture2D::EncodeToPNG(const std::string &file)
	{
		Image::EncodeToPNG(this, PIXEL_BITS_SIZE[m_format], file);
	}
}
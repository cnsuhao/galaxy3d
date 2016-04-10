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
			map->m_colors.resize(mip_count * 6);
		}
		else
		{
			map->m_colors.resize(6);
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

    void Cubemap::SetPixels(const char *colors, int size, int index)
    {
		if(index < m_colors.size())
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
        auto device = GraphicsDevice::GetInstance()->GetDevice();
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

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

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = static_cast<UINT>(m_width);
        desc.Height = static_cast<UINT>(m_height);
        desc.MipLevels = mip_count;
        desc.ArraySize = 6;
        desc.Format = TEXTURE_FORMATS[m_format];
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = mipmap ? (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) : D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = mipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE : D3D11_RESOURCE_MISC_TEXTURECUBE;

		int tex_count;
		if(mipmap && !m_mip_gen)
		{
			tex_count = mip_count * 6;
		}
		else
		{
			tex_count = 6;
		}

        D3D11_SUBRESOURCE_DATA *init_data = new D3D11_SUBRESOURCE_DATA[tex_count];
        ZeroMemory(init_data, sizeof(D3D11_SUBRESOURCE_DATA) * tex_count);
        char **buffer = new char *[tex_count];
        ZeroMemory(buffer, sizeof(char *) * tex_count);

        for(int k=0; k<tex_count; k++)
        {
            int bpp = Texture::PIXEL_BITS_SIZE[m_format];
            if(m_colors[k] != 0 && bpp > 0)
            {
                int bytes = bpp / 8;
				int mip_div = pow(2, k / 6);
				int w = m_width / mip_div;
				int h = m_height / mip_div;

                if(m_format == TextureFormat::RGB24)
                {
                    bytes = 4;
                    buffer[k] = new char[w * h * 4];
                    for(int i=0; i<h; i++)
                    {
                        for(int j=0; j<w; j++)
                        {
                            buffer[k][i * w * 4 + j * 4 + 0] = m_colors[k][i * w * 3 + j * 3 + 0];
                            buffer[k][i * w * 4 + j * 4 + 1] = m_colors[k][i * w * 3 + j * 3 + 1];
                            buffer[k][i * w * 4 + j * 4 + 2] = m_colors[k][i * w * 3 + j * 3 + 2];
                            buffer[k][i * w * 4 + j * 4 + 3] = (char) 0xff;
                        }
                    }

                    init_data[k].pSysMem = buffer[k];
                }
                else
                {
                    init_data[k].pSysMem = m_colors[k];
                }

                init_data[k].SysMemPitch = w * bytes;
            }
        }

        ID3D11Texture2D *tex;
        device->CreateTexture2D(&desc, mipmap ? NULL : init_data, &tex);

		ZeroMemory(&desc, sizeof(desc));
		tex->GetDesc(&desc);

		if(mipmap)
		{
			for(int i=0; i<tex_count; i++)
			{
				UINT sub = D3D11CalcSubresource(i / 6, i % 6, desc.MipLevels);
				context->UpdateSubresource(tex, sub, 0, init_data[i].pSysMem, init_data[i].SysMemPitch, 0);
			}
		}

        for(int i=0; i<tex_count; i++)
        {
            if(buffer[i] != NULL)
            {
                delete [] buffer[i];
                buffer[i] = 0;
            }
        }
		delete [] buffer;
		delete [] init_data;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
        memset(&srvd, 0, sizeof(srvd));
        srvd.Format = TEXTURE_FORMATS[m_format];
        srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvd.TextureCube.MipLevels = -1;
        srvd.TextureCube.MostDetailedMip = 0;

        device->CreateShaderResourceView(tex, &srvd, &m_texture_res);
        tex->Release();

		if(mipmap && m_mip_gen)
		{
			context->GenerateMips(m_texture_res);
		}

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
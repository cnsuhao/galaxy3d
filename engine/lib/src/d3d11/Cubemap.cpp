#include "Cubemap.h"
#include "Texture2D.h"

namespace Galaxy3D
{
    std::shared_ptr<Cubemap> Cubemap::LoadFromFile(const std::vector<std::string> &files, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode)
    {
        std::vector<std::shared_ptr<Texture2D>> texs;
        for(size_t i=0; i<files.size(); i++)
        {
            auto tex = Texture2D::LoadFromFile(files[i], filter_mode, wrap_mode, false);
            texs.push_back(tex);
        }

        auto map = std::shared_ptr<Cubemap>(new Cubemap(texs[0]->GetWidth(), filter_mode, wrap_mode));
        for(size_t i=0; i<files.size(); i++)
        {
            auto pixels = texs[i]->GetPixels();
            map->SetPixels(pixels, texs[i]->GetColorBufferSize(), i);
        }
        map->m_format = texs[0]->GetFormat();
        map->Apply();

        return map;
    }

    void Cubemap::SetPixels(const char *colors, int size, int index)
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

    void Cubemap::Apply()
    {
        auto device = GraphicsDevice::GetInstance()->GetDevice();
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = static_cast<UINT>(m_width);
        desc.Height = static_cast<UINT>(m_height);
        desc.MipLevels = 1;
        desc.ArraySize = 6;
        desc.Format = TEXTURE_FORMATS[m_format];
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA init_data[6];
        ZeroMemory(init_data, sizeof(init_data));
        char *buffer[6];
        ZeroMemory(buffer, sizeof(buffer));
        for(int k=0; k<6; k++)
        {
            int bpp = Texture::PIXEL_BITS_SIZE[m_format];
            if(m_colors != 0 && bpp > 0)
            {
                int bytes = bpp / 8;
                if(m_format == TextureFormat::RGB24)
                {
                    bytes = 4;
                    buffer[k] = new char[m_width * m_height * 4];
                    for(int i=0; i<m_height; i++)
                    {
                        for(int j=0; j<m_width; j++)
                        {
                            buffer[k][i * m_width * 4 + j * 4 + 0] = m_colors[k][i * m_width * 3 + j * 3 + 0];
                            buffer[k][i * m_width * 4 + j * 4 + 1] = m_colors[k][i * m_width * 3 + j * 3 + 1];
                            buffer[k][i * m_width * 4 + j * 4 + 2] = m_colors[k][i * m_width * 3 + j * 3 + 2];
                            buffer[k][i * m_width * 4 + j * 4 + 3] = (char) 0xff;
                        }
                    }

                    init_data[k].pSysMem = buffer[k];
                }
                else
                {
                    init_data[k].pSysMem = m_colors[k];
                }

                init_data[k].SysMemPitch = m_width * bytes;
            }
        }

        ID3D11Texture2D *tex;
        device->CreateTexture2D(&desc, init_data, &tex);

        for(int i=0; i<6; i++)
        {
            if(buffer[i] != NULL)
            {
                delete [] buffer[i];
                buffer[i] = 0;
            }
        }

        if(m_texture_res == NULL)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
            memset(&srvd, 0, sizeof(srvd));
            srvd.Format = TEXTURE_FORMATS[m_format];
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels = -1;

            device->CreateShaderResourceView(tex, &srvd, &m_texture_res);
            tex->Release();
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
}
#include "RenderTexture.h"

namespace Galaxy3D
{
    std::list<std::shared_ptr<RenderTexture>> RenderTexture::m_textures_idle;
    std::list<std::shared_ptr<RenderTexture>> RenderTexture::m_textures_using;

    std::shared_ptr<RenderTexture> RenderTexture::GetTemporary(
        int width,
        int height,
        RenderTextureFormat::Enum format,
        DepthBuffer::Enum depth)
    {
        std::shared_ptr<RenderTexture> texture;

        for(auto i=m_textures_idle.begin(); i!=m_textures_idle.end(); i++)
        {
            auto t = *i;
            if(	t->GetWidth() == width &&
                t->GetHeight() == height &&
                t->GetFormat() == format &&
                t->GetDepth() == depth)
            {
                texture = t;

                m_textures_idle.erase(i);
                m_textures_using.push_front(texture);
                break;
            }
        }

        if(!texture)
        {
            texture = Create(width, height, format, depth, FilterMode::Bilinear);

            m_textures_using.push_front(texture);
        }

        return texture;
    }

    void RenderTexture::ReleaseTemporary(const std::shared_ptr<RenderTexture> &temp)
    {
        for(auto i=m_textures_using.begin(); i!=m_textures_using.end(); i++)
        {
            if((*i) == temp)
            {
                m_textures_using.erase(i);
                m_textures_idle.push_front(temp);
                break;
            }
        }
    }

	void RenderTexture::SwapColorBuffer(std::shared_ptr<RenderTexture> &t1, std::shared_ptr<RenderTexture> &t2)
	{
	
	}

	std::shared_ptr<RenderTexture> RenderTexture::Create(
        int width,
        int height,
        RenderTextureFormat::Enum format,
        DepthBuffer::Enum depth,
        FilterMode::Enum filter_mode,
        TextureWrapMode::Enum wrap_mode)
    {
        auto texture = std::shared_ptr<RenderTexture>(new RenderTexture(width, height, format, depth, filter_mode, wrap_mode));
        texture->Create();

        return texture;
    }

	RenderTexture::~RenderTexture()
    {
        
    }

	void RenderTexture::Create()
	{
	
	}
}
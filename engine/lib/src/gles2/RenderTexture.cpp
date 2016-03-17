#include "RenderTexture.h"
#include "Debug.h"

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
        if(m_frame_buffer != 0)
		{
			glDeleteFramebuffers(1, &m_frame_buffer);
		}

		if(m_depth_texture != 0)
		{
			glDeleteTextures(1, &m_depth_texture);
		}

		if(m_color_texture != 0)
		{
			glDeleteTextures(1, &m_color_texture);
		}
    }

	void RenderTexture::Create()
	{
		if(m_format == RenderTextureFormat::RGBA32 || m_format == RenderTextureFormat::Depth)
		{
			GLuint color_texture;

			glGenTextures(1, &color_texture);
			glBindTexture(GL_TEXTURE_2D, color_texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Texture::FILTER_MODES[m_filter_mode]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture::FILTER_MODES[m_filter_mode]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Texture::ADDRESS_MODES[m_wrap_mode]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Texture::ADDRESS_MODES[m_wrap_mode]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			m_color_texture = color_texture;
		}

		if(m_format == RenderTextureFormat::Depth || m_depth != DepthBuffer::Depth_0)
		{
			GLuint depth_texture;

			glGenTextures(1, &depth_texture);
			glBindTexture(GL_TEXTURE_2D, depth_texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Texture::FILTER_MODES[m_filter_mode]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture::FILTER_MODES[m_filter_mode]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Texture::ADDRESS_MODES[m_wrap_mode]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Texture::ADDRESS_MODES[m_wrap_mode]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);

			m_depth_texture = depth_texture;
		}

		GLuint frame_buffer;

		glGenFramebuffers(1, &frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

		m_frame_buffer = frame_buffer;

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			Debug::Log("RenderTexture Create error:frame buffer status error %x", status);
		}
		else
		{
			Debug::Log("RenderTexture Create ok w:%d h:%d fmt:%d", m_width, m_height, m_format);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
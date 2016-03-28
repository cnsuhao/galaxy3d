#include "TextRenderer.h"
#include "GameObject.h"
#include "UICanvas.h"
#include "SpriteBatchRenderer.h"

namespace Galaxy3D
{
	TextRenderer::TextRenderer():
        m_color(1, 1, 1, 1),
		m_vertex_count(0),
        m_clip(false),
        m_clip_rect(),
        m_clip_soft(),
        m_dirty(true)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

		SetSharedMaterial(Material::Create("UI/Text"));
	}

	TextRenderer::~TextRenderer()
	{
		ReleaseBuffer();
		ReleaseBufferLabelImage();
	}

    void TextRenderer::Start()
    {
        auto canvas = GetGameObject()->GetComponentInParent<UICanvas>();

        if(canvas && m_anchor)
        {
			auto t = GetTransform();
            canvas->AnchorTransform(t, *m_anchor);
        }
    }

    void TextRenderer::SetAnchor(const Vector4 &anchor)
    {
        if(!m_anchor || *m_anchor != anchor)
        {
            m_anchor = std::make_shared<Vector4>(anchor);
            m_dirty = true;
        }
    }

	void TextRenderer::SetLabel(const std::shared_ptr<Label> &label)
	{
		if(m_label != label)
		{
			m_label = label;
            m_dirty = true;
		}
	}

	void TextRenderer::UpdateLabel()
	{
		if(!m_label)
		{
			return;
		}

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
		{
			if(m_label->GetVertexCount() > 0)
			{
				CreateVertexBuffer();
				CreateIndexBuffer();
			}

			m_vertex_count = m_label->GetVertexCount();
		}
		else
		{
			if(m_vertex_count != m_label->GetVertexCount())
			{
				ReleaseBuffer();

				if(m_label->GetVertexCount() > 0)
				{
					CreateVertexBuffer();
					CreateIndexBuffer();
				}

				m_vertex_count = m_label->GetVertexCount();
			}
			else
			{
				UpdateVertexBuffer();
			}
		}

		if(m_vertex_buffer_img.empty() || m_index_buffer_img.empty())
		{
			if(m_label->GetImageCount() > 0)
			{
				CreateVertexBufferLabelImage();
				CreateIndexBufferLabelImage();
			}
		}
		else
		{
			if(m_label->GetImageCount() != (int) m_vertex_buffer_img.size())
			{
				ReleaseBufferLabelImage();

				if(m_label->GetImageCount() > 0)
				{
					CreateVertexBufferLabelImage();
					CreateIndexBufferLabelImage();
				}
			}
			else
			{
				UpdateVertexBufferLabelImage();
			}
		}
	}

	void TextRenderer::Render(int material_index)
	{
		if(!m_label)
		{
			return;
		}

        if(m_dirty || m_label->IsDirty())
        {
            m_dirty = false;
            m_label->SetDirty(false);

            UpdateLabel();
        }

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
		{
            return;
		}

        auto mat = GetSharedMaterial();
        auto shader = mat->GetShader();
        auto pass = shader->GetPass(0);

        GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
        GraphicsDevice::GetInstance()->SetVertexBuffer(m_vertex_buffer, pass->vs);
        GraphicsDevice::GetInstance()->SetIndexBuffer(m_index_buffer, IndexType::UShort);

        auto camera = Camera::GetCurrent();
        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

        mat->SetMatrix("WorldViewProjection", wvp);
        mat->SetMainTexture(Label::GetFontTexture());
        mat->SetMainColor(m_color);
        if(m_clip && !m_clip_panel.expired())
        {
            auto wvp_clip = camera->GetViewProjectionMatrix() * m_clip_panel.lock()->GetTransform()->GetLocalToWorldMatrix();
            Vector3 min = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.x, m_clip_rect.w, 0));
            Vector3 max = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.z, m_clip_rect.y, 0));
            Vector4 rect(min.x, max.y, max.x, min.y);
            min = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.x + m_clip_soft.x, m_clip_rect.w + m_clip_soft.y, 0));
            max = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.z - m_clip_soft.x, m_clip_rect.y - m_clip_soft.y, 0));
            Vector4 soft(min.x, max.y, max.x, min.y);
            mat->SetVector("ClipRect", rect);
            mat->SetVector("ClipSoft", soft);
        }
        else
        {
            mat->SetVector("ClipRect", Vector4(-1, 1, 1, -1));
            mat->SetVector("ClipSoft", Vector4(-1, 1, 1, -1));
        }

        mat->ReadyPass(0);
        pass->rs->Apply();
        mat->ApplyPass(0);

        DrawIndexed(m_label->GetVertexCount() / 4 * 6, 0);

        GraphicsDevice::GetInstance()->ClearShaderResources();

		RenderLabelImage();
	}

	void TextRenderer::RenderLabelImage()
	{
		if(m_label->GetImageCount() == 0)
		{
			return;
		}

		if(!m_material_img)
		{
			m_material_img = Material::Create("UI/Sprite");
		}

		int c = 0;
		auto &lines = m_label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			for(size_t j=0; j<lines[i].image_items.size(); j++)
			{
				auto &img = lines[i].image_items[j];

				auto mat = m_material_img;
				auto shader = mat->GetShader();
				auto pass = shader->GetPass(0);

                GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
                GraphicsDevice::GetInstance()->SetVertexBuffer(m_vertex_buffer_img[c], pass->vs);
                GraphicsDevice::GetInstance()->SetIndexBuffer(m_index_buffer_img[c], IndexType::UShort);
		
				auto camera = Camera::GetCurrent();
				Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

				mat->SetMatrix("WorldViewProjection", wvp);
				auto tex = Label::GetRichImageTexture(img.name, img.image_index >> 16);
				mat->SetMainTexture(tex);
		
				mat->ReadyPass(0);
				pass->rs->Apply();
				mat->ApplyPass(0);

				DrawIndexed(6, 0);

				GraphicsDevice::GetInstance()->ClearShaderResources();

				if(img.image_count > 1)
				{
					img.image_index += 1400;
					if((img.image_index >> 16) >= img.image_count)
					{
						img.image_index = 0;
					}
				}

				c++;
			}
		}
	}

	void TextRenderer::CreateVertexBuffer()
	{
		int vertex_count = m_label->GetVertexCount();
		int buffer_size = sizeof(VertexUI) * vertex_count;
		char *buffer = (char *) malloc(buffer_size);

		Label::FillVertexBuffer(buffer, m_label, NULL);

        m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

		free(buffer);
	}

	void TextRenderer::UpdateVertexBuffer()
	{
		int vertex_count = m_label->GetVertexCount();
		int buffer_size = sizeof(VertexUI) * vertex_count;
		char *buffer = (char *) malloc(buffer_size);

		Label::FillVertexBuffer(buffer, m_label, NULL);

        GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);

		free(buffer);
	}

	void TextRenderer::CreateIndexBuffer()
	{
		int index_count = m_label->GetIndexCount();
		int buffer_size = sizeof(unsigned short) * index_count;
		char *buffer = (char *) malloc(buffer_size);
		char *p = buffer;

		Label::FillIndexBuffer(p, m_label);
		
        m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);

		free(buffer);
	}

	void TextRenderer::ReleaseBuffer()
	{
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
	}

	void TextRenderer::CreateVertexBufferLabelImage()
	{
		auto &lines = m_label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			auto &images = lines[i].image_items;
			for(size_t j=0; j<images.size(); j++)
			{
				int buffer_size = sizeof(VertexUI) * 4;
				char *buffer = (char *) malloc(buffer_size);

				Label::FillVertexBuffer(buffer, images[j], m_label, lines[i]);

                auto vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);
				
				free(buffer);

				m_vertex_buffer_img.push_back(vertex_buffer);
			}
		}
	}

	void TextRenderer::UpdateVertexBufferLabelImage()
	{
		int c = 0;
		auto &lines = m_label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			auto &images = lines[i].image_items;
			for(size_t j=0; j<images.size(); j++)
			{
				auto vertex_buffer = m_vertex_buffer_img[c];

				int buffer_size = sizeof(VertexUI) * 4;
				char *buffer = (char *) malloc(buffer_size);

				Label::FillVertexBuffer(buffer, images[j], m_label, lines[i]);

                GraphicsDevice::GetInstance()->UpdateBufferObject(vertex_buffer, buffer, buffer_size);

				free(buffer);
				c++;
			}
		}
	}

	void TextRenderer::CreateIndexBufferLabelImage()
	{
		auto &lines = m_label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			auto &images = lines[i].image_items;
			for(size_t j=0; j<images.size(); j++)
			{
				unsigned short *uv = &images[j].indices[0];
				int buffer_size = sizeof(unsigned short) * 6;
				char *buffer = (char *) malloc(buffer_size);
				char *p = buffer;

				memcpy(p, uv, buffer_size);

                auto index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);

				free(buffer);

				m_index_buffer_img.push_back(index_buffer);
			}
		}
	}

	void TextRenderer::ReleaseBufferLabelImage()
	{
		for(auto i : m_vertex_buffer_img)
		{
            GraphicsDevice::GetInstance()->ReleaseBufferObject(i);
		}
		m_vertex_buffer_img.clear();

		for(auto i : m_index_buffer_img)
		{
            GraphicsDevice::GetInstance()->ReleaseBufferObject(i);
		}
		m_index_buffer_img.clear();
	}
}
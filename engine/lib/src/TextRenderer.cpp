#include "TextRenderer.h"
#include "GameObject.h"
#include "UICanvas.h"
#include "SpriteBatchRenderer.h"

namespace Galaxy3D
{
	TextRenderer::TextRenderer():
        m_color(1, 1, 1, 1),
        m_clip(false),
        m_clip_rect(),
        m_clip_soft(),
		m_vertex_count(0),
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
            canvas->AnchorTransform(GetTransform(), *m_anchor);
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
			if(m_label->GetImageCount() != m_vertex_buffer_img.size())
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
        GraphicsDevice::GetInstance()->SetVertexBuffer(m_vertex_buffer, pass->vs->vertex_stride, 0);
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
                GraphicsDevice::GetInstance()->SetVertexBuffer(m_vertex_buffer_img[c], pass->vs->vertex_stride, 0);
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

	static void fill_vertex_buffer(char *buffer, const std::shared_ptr<Label> &label)
	{
		char *p = buffer;
		auto pivot = label->GetPivot();
		auto align = label->GetAlign();
		auto aw = label->GetWidthActual();
		auto ah = label->GetHeightActual();
		auto w = label->GetWidth();
		auto h = label->GetHeight();

		if(w < 0)
		{
			w = aw;
		}
		if(h < 0)
		{
			h = ah;
		}

		if(align == LabelAlign::Auto)
		{
			if(	pivot == LabelPivot::LeftTop ||
				pivot == LabelPivot::Left ||
				pivot == LabelPivot::LeftBottom)
			{
				align = LabelAlign::Left;
			}
			else if(
				pivot == LabelPivot::Top ||
				pivot == LabelPivot::Center ||
				pivot == LabelPivot::Bottom)
			{
				align = LabelAlign::Center;
			}
			else if(
				pivot == LabelPivot::RightTop ||
				pivot == LabelPivot::Right ||
				pivot == LabelPivot::RightBottom)
			{
				align = LabelAlign::Right;
			}
		}

		auto &lines = label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			auto &line = lines[i];
			auto &vertices = line.vertices;
			auto &colors = line.colors;
			auto &uv = line.uv;
			int vertex_count = vertices.size();

			for(int i=0; i<vertex_count; i++)
			{
				Vector3 pos = vertices[i];

				if(	pivot == LabelPivot::Top ||
					pivot == LabelPivot::Center ||
					pivot == LabelPivot::Bottom)
				{
					pos.x -= Mathf::Round(w * 0.5f);
				}

				if(	pivot == LabelPivot::RightTop ||
					pivot == LabelPivot::Right ||
					pivot == LabelPivot::RightBottom)
				{
					pos.x -= w;
				}

				if(	pivot == LabelPivot::Left ||
					pivot == LabelPivot::Center ||
					pivot == LabelPivot::Right)
				{
					pos.y += Mathf::Round(h * 0.5f);
				}

				if(	pivot == LabelPivot::LeftBottom ||
					pivot == LabelPivot::Bottom ||
					pivot == LabelPivot::RightBottom)
				{
					pos.y += h;
				}

				if(align == LabelAlign::Center)
				{
					pos.x += Mathf::Round((w - line.width) * 0.5f);
				}
				else if(align == LabelAlign::Right)
				{
					pos.x += (w - line.width);
				}

				memcpy(p, &pos, sizeof(Vector3));
				p += sizeof(Vector3);

				Color c = colors[i];
				memcpy(p, &c, sizeof(Color));
				p += sizeof(Color);

				Vector2 v1 = uv[i];
				memcpy(p, &v1, sizeof(Vector2));
				p += sizeof(Vector2);
			}
		}
	}

	static void fill_vertex_buffer(char *buffer, LabelImageItem &item, const std::shared_ptr<Label> &label, const LabelLine &line)
	{
		char *p = buffer;
		Vector2 *vertices = &item.vertices[0];
		Vector2 *uv = &item.uv[0];
		auto pivot = label->GetPivot();
		auto align = label->GetAlign();
		auto aw = label->GetWidthActual();
		auto ah = label->GetHeightActual();
		auto w = label->GetWidth();
		auto h = label->GetHeight();

		if(w < 0)
		{
			w = aw;
		}
		if(h < 0)
		{
			h = ah;
		}

		if(align == LabelAlign::Auto)
		{
			if(	pivot == LabelPivot::LeftTop ||
				pivot == LabelPivot::Left ||
				pivot == LabelPivot::LeftBottom)
			{
				align = LabelAlign::Left;
			}
			else if(
				pivot == LabelPivot::Top ||
				pivot == LabelPivot::Center ||
				pivot == LabelPivot::Bottom)
			{
				align = LabelAlign::Center;
			}
			else if(
				pivot == LabelPivot::RightTop ||
				pivot == LabelPivot::Right ||
				pivot == LabelPivot::RightBottom)
			{
				align = LabelAlign::Right;
			}
		}

		for(int i=0; i<4; i++)
		{
			Vector3 pos = vertices[i];

			if(	pivot == LabelPivot::Top ||
				pivot == LabelPivot::Center ||
				pivot == LabelPivot::Bottom)
			{
				pos.x -= Mathf::Round(w * 0.5f);
			}

			if(	pivot == LabelPivot::RightTop ||
				pivot == LabelPivot::Right ||
				pivot == LabelPivot::RightBottom)
			{
				pos.x -= w;
			}

			if(	pivot == LabelPivot::Left ||
				pivot == LabelPivot::Center ||
				pivot == LabelPivot::Right)
			{
				pos.y += Mathf::Round(h * 0.5f);
			}

			if(	pivot == LabelPivot::LeftBottom ||
				pivot == LabelPivot::Bottom ||
				pivot == LabelPivot::RightBottom)
			{
				pos.y += h;
			}

			if(align == LabelAlign::Center)
			{
				pos.x += Mathf::Round((w - line.width) * 0.5f);
			}
			else if(align == LabelAlign::Right)
			{
				pos.x += (w - line.width);
			}

			memcpy(p, &pos, sizeof(Vector3));
			p += sizeof(Vector3);

			Color c(1, 1, 1, 1);
			memcpy(p, &c, sizeof(Color));
			p += sizeof(Color);

			Vector2 v1 = uv[i];
			memcpy(p, &v1, sizeof(Vector2));
			p += sizeof(Vector2);
		}
	}

	void TextRenderer::CreateVertexBuffer()
	{
		int vertex_count = m_label->GetVertexCount();
		int buffer_size = sizeof(VertexUI) * vertex_count;
		char *buffer = (char *) malloc(buffer_size);

		fill_vertex_buffer(buffer, m_label);

        m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

		free(buffer);
	}

	void TextRenderer::UpdateVertexBuffer()
	{
		int vertex_count = m_label->GetVertexCount();
		int buffer_size = sizeof(VertexUI) * vertex_count;
		char *buffer = (char *) malloc(buffer_size);

		fill_vertex_buffer(buffer, m_label);

        GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);

		free(buffer);
	}

	void TextRenderer::CreateIndexBuffer()
	{
		int index_count = m_label->GetVertexCount() / 4 * 6;
		int buffer_size = sizeof(unsigned short) * index_count;
		char *buffer = (char *) malloc(buffer_size);
		char *p = buffer;

		auto &lines = m_label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			auto &indices = lines[i].indices;
			int size = sizeof(unsigned short) * indices.size();
			
			if(size > 0)
			{
				memcpy(p, &indices[0], size);
				p += size;
			}
		}
		
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

				fill_vertex_buffer(buffer, images[j], m_label, lines[i]);

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

				fill_vertex_buffer(buffer, images[j], m_label, lines[i]);

                GraphicsDevice::GetInstance()->UpdateBufferObject(vertex_buffer, buffer, buffer_size);

				free(buffer);
				c++;
			}
		}
	}

	void TextRenderer::CreateIndexBufferLabelImage()
	{
		int c = 0;
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
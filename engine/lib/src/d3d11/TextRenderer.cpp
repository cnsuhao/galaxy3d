#include "TextRenderer.h"

namespace Galaxy3D
{
	TextRenderer::TextRenderer():
		m_vertex_buffer(NULL),
		m_index_buffer(NULL),
		m_vertex_count(0)
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

	void TextRenderer::SetLabel(const std::shared_ptr<Label> &label)
	{
		if(m_label != label)
		{
			m_label = label;
		}
	}

	void TextRenderer::UpdateLabel()
	{
		if(!m_label)
		{
			return;
		}

		if(m_vertex_buffer == NULL || m_index_buffer == NULL)
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

		if(m_vertex_buffer != NULL && m_index_buffer != NULL)
		{
			auto mat = GetSharedMaterial();
			auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
			auto shader = mat->GetShader();
			auto pass = shader->GetPass(0);

			context->IASetInputLayout(pass->vs->input_layout);

			UINT stride = pass->vs->vertex_stride;
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);
			context->IASetIndexBuffer(m_index_buffer, DXGI_FORMAT_R16_UINT, 0);
		
			auto camera = Camera::GetCurrent();
			Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

			mat->SetMatrix("WorldViewProjection", wvp);
			mat->SetMainTexture(Label::GetFontTexture());
		
			mat->ReadyPass(0);
			pass->rs->Apply();
			mat->ApplyPass(0);

			DrawIndexed(m_label->GetVertexCount() / 4 * 6, 0);

			GraphicsDevice::GetInstance()->ClearShaderResources();
		}

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
				auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
				auto shader = mat->GetShader();
				auto pass = shader->GetPass(0);

				context->IASetInputLayout(pass->vs->input_layout);

				UINT stride = pass->vs->vertex_stride;
				UINT offset = 0;
				context->IASetVertexBuffers(0, 1, &m_vertex_buffer_img[c], &stride, &offset);
				context->IASetIndexBuffer(m_index_buffer_img[c], DXGI_FORMAT_R16_UINT, 0);
		
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
		auto color = label->GetColor();
		auto pivot = label->GetPivot();
		auto align = label->GetAlign();
		auto aw = label->GetWidthActual();
		auto ah = label->GetHeightActual();
		auto w = label->GetWidth();
		auto h = label->GetHeight();
		float ppu = label->GetPixelsPerUnit();
		float v_ppu = 1.0f / ppu;

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
					pos.x -= Mathf::Round(w * 0.5f) * v_ppu;
				}

				if(	pivot == LabelPivot::RightTop ||
					pivot == LabelPivot::Right ||
					pivot == LabelPivot::RightBottom)
				{
					pos.x -= w * v_ppu;
				}

				if(	pivot == LabelPivot::Left ||
					pivot == LabelPivot::Center ||
					pivot == LabelPivot::Right)
				{
					pos.y += Mathf::Round(h * 0.5f) * v_ppu;
				}

				if(	pivot == LabelPivot::LeftBottom ||
					pivot == LabelPivot::Bottom ||
					pivot == LabelPivot::RightBottom)
				{
					pos.y += h * v_ppu;
				}

				if(align == LabelAlign::Center)
				{
					pos.x += Mathf::Round((w - line.width) * 0.5f) * v_ppu;
				}
				else if(align == LabelAlign::Right)
				{
					pos.x += (w - line.width) * v_ppu;
				}

				memcpy(p, &pos, sizeof(Vector3));
				p += sizeof(Vector3);

				Color c = colors[i] * color;
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
		auto color = label->GetColor();
		auto pivot = label->GetPivot();
		auto align = label->GetAlign();
		auto aw = label->GetWidthActual();
		auto ah = label->GetHeightActual();
		auto w = label->GetWidth();
		auto h = label->GetHeight();
		float ppu = label->GetPixelsPerUnit();
		float v_ppu = 1.0f / ppu;

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
				pos.x -= Mathf::Round(w * 0.5f) * v_ppu;
			}

			if(	pivot == LabelPivot::RightTop ||
				pivot == LabelPivot::Right ||
				pivot == LabelPivot::RightBottom)
			{
				pos.x -= w * v_ppu;
			}

			if(	pivot == LabelPivot::Left ||
				pivot == LabelPivot::Center ||
				pivot == LabelPivot::Right)
			{
				pos.y += Mathf::Round(h * 0.5f) * v_ppu;
			}

			if(	pivot == LabelPivot::LeftBottom ||
				pivot == LabelPivot::Bottom ||
				pivot == LabelPivot::RightBottom)
			{
				pos.y += h * v_ppu;
			}

			if(align == LabelAlign::Center)
			{
				pos.x += Mathf::Round((w - line.width) * 0.5f) * v_ppu;
			}
			else if(align == LabelAlign::Right)
			{
				pos.x += (w - line.width) * v_ppu;
			}

			memcpy(p, &pos, sizeof(Vector3));
			p += sizeof(Vector3);

			Color c = color;
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

		bool dynamic = true;

		auto device = GraphicsDevice::GetInstance()->GetDevice();

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = buffer_size;

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = buffer;
		HRESULT hr = device->CreateBuffer(&bd, &data, &m_vertex_buffer);
		
		free(buffer);
	}

	void TextRenderer::UpdateVertexBuffer()
	{
		int vertex_count = m_label->GetVertexCount();
		int buffer_size = sizeof(VertexUI) * vertex_count;
		char *buffer = (char *) malloc(buffer_size);

		fill_vertex_buffer(buffer, m_label);

		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		D3D11_MAPPED_SUBRESOURCE dms;
		ZeroMemory(&dms, sizeof(dms));
		context->Map(m_vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
		memcpy(dms.pData, &buffer[0], buffer_size);
		context->Unmap(m_vertex_buffer, 0);

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
		
		auto device = GraphicsDevice::GetInstance()->GetDevice();

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.CPUAccessFlags = 0;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.ByteWidth = buffer_size;

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = buffer;
		HRESULT hr = device->CreateBuffer(&bd, &data, &m_index_buffer);

		free(buffer);
	}

	void TextRenderer::ReleaseBuffer()
	{
		SAFE_RELEASE(m_vertex_buffer);
		SAFE_RELEASE(m_index_buffer);
	}

	void TextRenderer::CreateVertexBufferLabelImage()
	{
		auto &lines = m_label->GetLines();
		for(size_t i=0; i<lines.size(); i++)
		{
			auto &images = lines[i].image_items;
			for(size_t j=0; j<images.size(); j++)
			{
				ID3D11Buffer *vertex_buffer = NULL;

				int buffer_size = sizeof(VertexUI) * 4;
				char *buffer = (char *) malloc(buffer_size);

				fill_vertex_buffer(buffer, images[j], m_label, lines[i]);

				bool dynamic = true;

				auto device = GraphicsDevice::GetInstance()->GetDevice();

				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.ByteWidth = buffer_size;

				D3D11_SUBRESOURCE_DATA data;
				ZeroMemory(&data, sizeof(data));
				data.pSysMem = buffer;
				HRESULT hr = device->CreateBuffer(&bd, &data, &vertex_buffer);
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
				ID3D11Buffer *vertex_buffer = m_vertex_buffer_img[c];

				int buffer_size = sizeof(VertexUI) * 4;
				char *buffer = (char *) malloc(buffer_size);

				fill_vertex_buffer(buffer, images[j], m_label, lines[i]);

				auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

				D3D11_MAPPED_SUBRESOURCE dms;
				ZeroMemory(&dms, sizeof(dms));
				context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
				memcpy(dms.pData, &buffer[0], buffer_size);
				context->Unmap(vertex_buffer, 0);

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
				ID3D11Buffer *index_buffer = NULL;

				unsigned short *uv = &images[j].indices[0];
				int buffer_size = sizeof(unsigned short) * 6;
				char *buffer = (char *) malloc(buffer_size);
				char *p = buffer;

				memcpy(p, uv, buffer_size);

				auto device = GraphicsDevice::GetInstance()->GetDevice();

				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = D3D11_USAGE_IMMUTABLE;
				bd.CPUAccessFlags = 0;
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bd.ByteWidth = buffer_size;

				D3D11_SUBRESOURCE_DATA data;
				ZeroMemory(&data, sizeof(data));
				data.pSysMem = buffer;
				HRESULT hr = device->CreateBuffer(&bd, &data, &index_buffer);
				free(buffer);

				m_index_buffer_img.push_back(index_buffer);
			}
		}
	}

	void TextRenderer::ReleaseBufferLabelImage()
	{
		for(auto i : m_vertex_buffer_img)
		{
			SAFE_RELEASE(i);
		}
		m_vertex_buffer_img.clear();

		for(auto i : m_index_buffer_img)
		{
			SAFE_RELEASE(i);
		}
		m_index_buffer_img.clear();
	}
}
#include "TextRenderer.h"

namespace Galaxy3D
{
	TextRenderer::TextRenderer():
		m_vertex_buffer(nullptr),
		m_index_buffer(nullptr),
		m_vertex_count(0)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

		SetSharedMaterial(Material::Create("Text"));
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

		if(m_vertex_buffer == nullptr || m_index_buffer == nullptr)
		{
			if(!m_label->GetVertices().empty())
			{
				CreateVertexBuffer();
				CreateIndexBuffer();
			}

			m_vertex_count = m_label->GetVertices().size();
		}
		else
		{
			if(m_vertex_count != m_label->GetVertices().size())
			{
				ReleaseBuffer();

				if(!m_label->GetVertices().empty())
				{
					CreateVertexBuffer();
					CreateIndexBuffer();
				}

				m_vertex_count = m_label->GetVertices().size();
			}
			else
			{
				UpdateVertexBuffer();
			}
		}

		auto &imgs = m_label->GetImageItems();
		if(m_vertex_buffer_img.empty() || m_index_buffer_img.empty())
		{
			if(!imgs.empty())
			{
				CreateVertexBufferLabelImage();
				CreateIndexBufferLabelImage();
			}
		}
		else
		{
			if(imgs.size() != m_vertex_buffer_img.size())
			{
				ReleaseBufferLabelImage();

				if(!imgs.empty())
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

	void TextRenderer::Render()
	{
		if(!m_label)
		{
			return;
		}

		if(m_vertex_buffer != nullptr && m_index_buffer != nullptr)
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

			context->DrawIndexed(m_label->GetIndices().size(), 0, 0);

			GraphicsDevice::GetInstance()->ClearShaderResources();
		}
		else
		{
			DecDrawCall();
		}

		RenderLabelImage();
	}

	void TextRenderer::RenderLabelImage()
	{
		auto &images = m_label->GetImageItems();

		if(images.empty())
		{
			return;
		}

		if(!m_material_img)
		{
			m_material_img = Material::Create("Sprite");
		}
		
		for(size_t i=0; i<images.size(); i++)
		{
			auto mat = m_material_img;
			auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
			auto shader = mat->GetShader();
			auto pass = shader->GetPass(0);

			context->IASetInputLayout(pass->vs->input_layout);

			UINT stride = pass->vs->vertex_stride;
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, &m_vertex_buffer_img[i], &stride, &offset);
			context->IASetIndexBuffer(m_index_buffer_img[i], DXGI_FORMAT_R16_UINT, 0);
		
			auto camera = Camera::GetCurrent();
			Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

			mat->SetMatrix("WorldViewProjection", wvp);
			auto tex = Label::GetRichImageTexture(images[i].name, images[i].image_index >> 16);
			mat->SetMainTexture(tex);
		
			mat->ReadyPass(0);
			pass->rs->Apply();
			mat->ApplyPass(0);

			context->DrawIndexed(6, 0, 0);

			GraphicsDevice::GetInstance()->ClearShaderResources();

			AddDrawCall();

			if(images[i].image_count > 1)
			{
				images[i].image_index += 1400;
				if((images[i].image_index >> 16) >= images[i].image_count)
				{
					images[i].image_index = 0;
				}
			}
		}
	}

	static void fill_vertex_buffer(char *buffer, const std::shared_ptr<Label> &label)
	{
		char *p = buffer;
		auto &vertices = label->GetVertices();
		auto &colors = label->GetColors();
		auto &uv = label->GetUV();
		auto color = label->GetColor();
		int vertex_count = vertices.size();
		auto pivot = label->GetPivot();
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

			memcpy(p, &pos, sizeof(Vector3));
			p += sizeof(Vector3);

			Vector3 n;
			memcpy(p, &n, sizeof(Vector3));
			p += sizeof(Vector3);

			Vector4 t;
			memcpy(p, &t, sizeof(Vector4));
			p += sizeof(Vector4);

			Color c = colors[i] * color;
			memcpy(p, &c, sizeof(Color));
			p += sizeof(Color);

			Vector2 v1 = uv[i];
			memcpy(p, &v1, sizeof(Vector2));
			p += sizeof(Vector2);

			Vector2 v2;
			memcpy(p, &v2, sizeof(Vector2));
			p += sizeof(Vector2);
		}
	}

	static void fill_vertex_buffer(char *buffer, LabelImageItem &item, const std::shared_ptr<Label> &label)
	{
		char *p = buffer;
		Vector2 *vertices = &item.vertices[0];
		Vector2 *uv = &item.uv[0];
		auto color = label->GetColor();
		auto pivot = label->GetPivot();
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

			memcpy(p, &pos, sizeof(Vector3));
			p += sizeof(Vector3);

			Vector3 n;
			memcpy(p, &n, sizeof(Vector3));
			p += sizeof(Vector3);

			Vector4 t;
			memcpy(p, &t, sizeof(Vector4));
			p += sizeof(Vector4);

			Color c = color;
			memcpy(p, &c, sizeof(Color));
			p += sizeof(Color);

			Vector2 v1 = uv[i];
			memcpy(p, &v1, sizeof(Vector2));
			p += sizeof(Vector2);

			Vector2 v2;
			memcpy(p, &v2, sizeof(Vector2));
			p += sizeof(Vector2);
		}
	}

	void TextRenderer::CreateVertexBuffer()
	{
		int vertex_count = m_label->GetVertices().size();
		int buffer_size = sizeof(VertexMesh) * vertex_count;
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
		int vertex_count = m_label->GetVertices().size();
		int buffer_size = sizeof(VertexMesh) * vertex_count;
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
		auto &indices = m_label->GetIndices();
		int index_count = indices.size();
		int buffer_size = sizeof(unsigned short) * index_count;
		char *buffer = (char *) malloc(buffer_size);

		memcpy(buffer, &indices[0], buffer_size);
		
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
		auto &images = m_label->GetImageItems();
		for(size_t i=0; i<images.size(); i++)
		{
			ID3D11Buffer *vertex_buffer = nullptr;

			int buffer_size = sizeof(VertexMesh) * 4;
			char *buffer = (char *) malloc(buffer_size);

			fill_vertex_buffer(buffer, images[i], m_label);

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

	void TextRenderer::UpdateVertexBufferLabelImage()
	{
		auto &images = m_label->GetImageItems();
		for(size_t i=0; i<images.size(); i++)
		{
			ID3D11Buffer *vertex_buffer = m_vertex_buffer_img[i];

			int buffer_size = sizeof(VertexMesh) * 4;
			char *buffer = (char *) malloc(buffer_size);

			fill_vertex_buffer(buffer, images[i], m_label);

			auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

			D3D11_MAPPED_SUBRESOURCE dms;
			ZeroMemory(&dms, sizeof(dms));
			context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
			memcpy(dms.pData, &buffer[0], buffer_size);
			context->Unmap(vertex_buffer, 0);

			free(buffer);
		}
	}

	void TextRenderer::CreateIndexBufferLabelImage()
	{
		auto &images = m_label->GetImageItems();
		for(size_t i=0; i<images.size(); i++)
		{
			ID3D11Buffer *index_buffer = nullptr;

			unsigned short *uv = &images[i].indices[0];
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
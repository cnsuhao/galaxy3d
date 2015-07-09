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
		ReleaseLabelImageBuffer();
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
	}

	void TextRenderer::Render()
	{
		if(!m_label)
		{
			return;
		}

		if(m_vertex_buffer == nullptr || m_index_buffer == nullptr)
		{
			return;
		}

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

		RenderLabelImage();
	}

	void TextRenderer::RenderLabelImage()
	{
		auto images = m_label->GetImageItems();
		for(auto i : images)
		{
			AddDrawCall();
		}
	}

	void TextRenderer::ReleaseLabelImageBuffer()
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

	static void fill_vertex_buffer(char *buffer, const std::shared_ptr<Label> &label)
	{
		char *p = buffer;
		auto &vertices = label->GetVertices();
		auto &colors = label->GetColors();
		auto &uv = label->GetUV();
		auto color = label->GetColor();
		int vertex_count = vertices.size();

		for(int i=0; i<vertex_count; i++)
		{
			Vector3 pos = vertices[i];
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
}
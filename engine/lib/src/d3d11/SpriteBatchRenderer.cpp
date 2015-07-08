#include "SpriteBatchRenderer.h"

namespace Galaxy3D
{
	SpriteBatchRenderer::SpriteBatchRenderer():
		m_color(1, 1, 1, 1),
		m_vertex_buffer(nullptr),
		m_index_buffer(nullptr)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

		SetSharedMaterial(Material::Create("Sprite"));
	}

	SpriteBatchRenderer::~SpriteBatchRenderer()
	{
		Release();
	}

	void SpriteBatchRenderer::AddSprite(const std::shared_ptr<SpriteNode> &sprite)
	{
		if(	m_sprites.empty() ||
			(m_sprites.front()->GetSprite() && sprite && m_sprites.front()->GetSprite()->GetTexture() == sprite->GetSprite()->GetTexture()))
		{
			m_sprites.push_back(sprite);
			m_sprites.sort(SpriteNode::Less);
		}
	}

	void SpriteBatchRenderer::RemoveSprite(const std::shared_ptr<SpriteNode> &sprite)
	{
		for(auto i=m_sprites.begin(); i!=m_sprites.end(); i++)
		{
			if((*i) == sprite)
			{
				m_sprites.erase(i);
				break;
			}
		}
	}

	void SpriteBatchRenderer::Render()
	{
		if(m_sprites.empty())
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
		Matrix4x4 wvp = camera->GetViewProjectionMatrix() * Matrix4x4::Identity();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(m_sprites.front()->GetSprite()->GetTexture());
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

		context->DrawIndexed(6 * m_sprites.size(), 0, 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}

	void SpriteBatchRenderer::UpdateSprites()
	{
		if(m_sprites.empty())
		{
			return;
		}

		if(m_vertex_buffer == nullptr || m_index_buffer == nullptr)
		{
			CreateVertexBuffer();
			CreateIndexBuffer();
		}
		else
		{
			if(m_sprites_cache.size() != m_sprites.size())
			{
				Release();

				CreateVertexBuffer();
				CreateIndexBuffer();
			}
			else
			{
				UpdateVertexBuffer();
			}
		}

		m_sprites_cache = m_sprites;
	}

	void SpriteBatchRenderer::Release()
	{
		SAFE_RELEASE(m_vertex_buffer);
		SAFE_RELEASE(m_index_buffer);
	}

	static void fill_vertex_buffer(char *buffer, const std::shared_ptr<SpriteNode> &sprite, const Color &color)
	{
		char *p = buffer;
		auto s = sprite->GetSprite();
		auto mat = sprite->GetTransform()->GetLocalToWorldMatrix();
		Vector2 *vertices = s->GetVertices();
		Vector2 *uv = s->GetUV();
		Color c = sprite->GetColor() * color;

		for(int i=0; i<4; i++)
		{
			Vector3 pos = mat.MultiplyPoint3x4(vertices[i]);
			memcpy(p, &pos, sizeof(Vector3));
			p += sizeof(Vector3);

			Vector3 n;
			memcpy(p, &n, sizeof(Vector3));
			p += sizeof(Vector3);

			Vector4 t;
			memcpy(p, &t, sizeof(Vector4));
			p += sizeof(Vector4);

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

	void SpriteBatchRenderer::CreateVertexBuffer()
	{
		int sprite_count = m_sprites.size();
		int buffer_size = sizeof(VertexMesh) * 4 * sprite_count;
		char *buffer = (char *) malloc(buffer_size);

		auto j = m_sprites.begin();
		for(int i=0; i<sprite_count; i++)
		{
			fill_vertex_buffer(buffer + i * (sizeof(VertexMesh) * 4), *j, m_color);
			j++;
		}

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

	void SpriteBatchRenderer::UpdateVertexBuffer()
	{
		int sprite_count = m_sprites.size();
		int buffer_size = sizeof(VertexMesh) * 4 * sprite_count;
		char *buffer = (char *) malloc(buffer_size);

		auto j = m_sprites.begin();
		for(int i=0; i<sprite_count; i++)
		{
			fill_vertex_buffer(buffer + i * (sizeof(VertexMesh) * 4), *j, m_color);
			j++;
		}

		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		D3D11_MAPPED_SUBRESOURCE dms;
		ZeroMemory(&dms, sizeof(dms));
		context->Map(m_vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
		memcpy(dms.pData, &buffer[0], buffer_size);
		context->Unmap(m_vertex_buffer, 0);

		free(buffer);
	}
	
	void SpriteBatchRenderer::CreateIndexBuffer()
	{
		int sprite_count = m_sprites.size();
		int buffer_size = sizeof(unsigned short) * 6 * sprite_count;
		char *buffer = (char *) malloc(buffer_size);
		unsigned short *p = (unsigned short *) buffer;

		auto j = m_sprites.begin();
		for(int i=0; i<sprite_count; i++)
		{
			unsigned short *uv = (*j)->GetSprite()->GetIndices();
			for(int k=0; k<6; k++)
			{
				p[i * 6 + k] = uv[k] + i * 4;
			}

			j++;
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
}
#include "SpriteBatchRenderer.h"

namespace Galaxy3D
{
	SpriteBatchRenderer::SpriteBatchRenderer():
		m_color(1, 1, 1, 1)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

		SetSharedMaterial(Material::Create("UI/Sprite"));
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

	void SpriteBatchRenderer::Render(int material_index)
	{
		if(m_sprites.empty())
		{
			return;
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
		Matrix4x4 wvp = camera->GetViewProjectionMatrix() * Matrix4x4::Identity();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(m_sprites.front()->GetSprite()->GetTexture());
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

        DrawIndexed(6 * m_sprites.size(), 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}

	void SpriteBatchRenderer::UpdateSprites()
	{
		if(m_sprites.empty())
		{
			return;
		}

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
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
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
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

			memcpy(p, &c, sizeof(Color));
			p += sizeof(Color);

			Vector2 v1 = uv[i];
			memcpy(p, &v1, sizeof(Vector2));
			p += sizeof(Vector2);
		}
	}

	void SpriteBatchRenderer::CreateVertexBuffer()
	{
		int sprite_count = m_sprites.size();
		int buffer_size = sizeof(VertexUI) * 4 * sprite_count;
		char *buffer = (char *) malloc(buffer_size);

		auto j = m_sprites.begin();
		for(int i=0; i<sprite_count; i++)
		{
			fill_vertex_buffer(buffer + i * (sizeof(VertexUI) * 4), *j, m_color);
			j++;
		}

        m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

		free(buffer);
	}

	void SpriteBatchRenderer::UpdateVertexBuffer()
	{
		int sprite_count = m_sprites.size();
		int buffer_size = sizeof(VertexUI) * 4 * sprite_count;
		char *buffer = (char *) malloc(buffer_size);

		auto j = m_sprites.begin();
		for(int i=0; i<sprite_count; i++)
		{
			fill_vertex_buffer(buffer + i * (sizeof(VertexUI) * 4), *j, m_color);
			j++;
		}

        GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);

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
		
        m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);

		free(buffer);
	}
}
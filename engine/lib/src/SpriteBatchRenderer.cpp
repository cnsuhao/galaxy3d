#include "SpriteBatchRenderer.h"
#include "GameObject.h"
#include "UICanvas.h"

namespace Galaxy3D
{
    static int get_sprites_vertex_count(std::list<std::shared_ptr<SpriteNode>> &list)
    {
        int vertex_count = 0;
        for(auto &i : list)
        {
            vertex_count += i->GetSprite()->GetVertexCount();
        }

        return vertex_count;
    }

    static int get_sprites_index_count(std::list<std::shared_ptr<SpriteNode>> &list)
    {
        int index_count = 0;
        for(auto &i : list)
        {
            index_count += i->GetSprite()->GetIndexCount();
        }

        return index_count;
    }

	SpriteBatchRenderer::SpriteBatchRenderer():
		m_color(1, 1, 1, 1),
        m_dirty(true)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

        auto mat = Material::Create("UI/Sprite");
		SetSharedMaterial(mat);
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

            sprite->SetBatch(std::dynamic_pointer_cast<SpriteBatchRenderer>(GetComponentPtr()));

            m_dirty = true;
		}
	}

	void SpriteBatchRenderer::RemoveSprite(const std::shared_ptr<SpriteNode> &sprite)
	{
		for(auto i=m_sprites.begin(); i!=m_sprites.end(); i++)
		{
			if((*i) == sprite)
			{
				m_sprites.erase(i);

                m_dirty = true;
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

        if(IsDirty())
        {
            m_dirty = false;
            for(auto &i : m_sprites)
            {
                i->SetDirty(false);
            }

            UpdateSprites();
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
        mat->SetMainColor(m_color);
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

        DrawIndexed(get_sprites_index_count(m_sprites), 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}

    bool SpriteBatchRenderer::IsDirty()
    {
        bool any_sprite_dirty = false;

        for(auto &i : m_sprites)
        {
            if(i->IsDirty() || i->GetSprite()->IsDirdy())
            {
                any_sprite_dirty = true;
                break;
            }
        }

        return any_sprite_dirty || m_dirty;
    }

	void SpriteBatchRenderer::UpdateSprites()
	{
		if(m_sprites.empty())
		{
			return;
		}

        for(auto &i : m_sprites)
        {
            i->AnchorTransform();
        }

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
		{
			CreateVertexBuffer();
			CreateIndexBuffer();
		}
		else
		{
            if( m_sprites_cache.size() != m_sprites.size() ||
                get_sprites_vertex_count(m_sprites_cache) != get_sprites_vertex_count(m_sprites) ||
                get_sprites_index_count(m_sprites_cache) != get_sprites_index_count(m_sprites))
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

	static int fill_vertex_buffer(char *buffer, const std::shared_ptr<SpriteNode> &sprite)
	{
		char *p = buffer;
		auto s = sprite->GetSprite();
		auto mat_world = sprite->GetTransform()->GetLocalToWorldMatrix();
        int vertex_count = s->GetVertexCount();
		Vector2 *vertices = s->GetVertices();
		Vector2 *uv = s->GetUV();
		Color c = sprite->GetColor();

		for(int i=0; i<vertex_count; i++)
		{
			Vector3 pos = mat_world.MultiplyPoint3x4(vertices[i]);
			memcpy(p, &pos, sizeof(Vector3));
			p += sizeof(Vector3);

			memcpy(p, &c, sizeof(Color));
			p += sizeof(Color);

			Vector2 v1 = uv[i];
			memcpy(p, &v1, sizeof(Vector2));
			p += sizeof(Vector2);
		}

        return p - buffer;
	}

	void SpriteBatchRenderer::CreateVertexBuffer()
	{
        int vertex_count = get_sprites_vertex_count(m_sprites);
        if(vertex_count > 0)
        {
            int buffer_size = sizeof(VertexUI) * vertex_count;
            char *buffer = (char *) malloc(buffer_size);

            char *p = buffer;
            for(auto &i : m_sprites)
            {
                p += fill_vertex_buffer(p, i);
            }

            m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

            free(buffer);
        }
	}

	void SpriteBatchRenderer::UpdateVertexBuffer()
	{
        int vertex_count = get_sprites_vertex_count(m_sprites);
        if(vertex_count)
        {
            int buffer_size = sizeof(VertexUI) * vertex_count;
            char *buffer = (char *) malloc(buffer_size);

            char *p = buffer;
            for(auto &i : m_sprites)
            {
                p += fill_vertex_buffer(p, i);
            }

            GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);

            free(buffer);
        }
	}
	
	void SpriteBatchRenderer::CreateIndexBuffer()
	{
        int index_count = get_sprites_index_count(m_sprites);
        if(index_count > 0)
        {
            int buffer_size = sizeof(unsigned short) * index_count;
            char *buffer = (char *) malloc(buffer_size);
            unsigned short *p = (unsigned short *) buffer;

            int vertex_count = 0;
            for(auto &i : m_sprites)
            {
                int index_count = i->GetSprite()->GetIndexCount();
                unsigned short *uv = i->GetSprite()->GetIndices();

                for(int j=0; j<index_count; j++)
                {
                    unsigned short index = uv[j] + vertex_count;
                    *p = index;
                    p++;
                }

                vertex_count += i->GetSprite()->GetVertexCount();
            }

            m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);

            free(buffer);
        }
	}
}
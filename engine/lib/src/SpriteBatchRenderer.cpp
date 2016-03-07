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
            if(i->GetGameObject()->IsActiveInHierarchy() && i->IsEnable())
            {
                index_count += i->GetSprite()->GetIndexCount();
            }
        }

        return index_count;
    }

	SpriteBatchRenderer::SpriteBatchRenderer():
		m_color(1, 1, 1, 1),
        m_clip(false),
        m_clip_rect(),
        m_clip_soft(),
        m_dirty(true),
        m_vertex_count_old(-1),
        m_index_count_old(-1)
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
			(m_sprites.front()->GetSprite() && sprite && sprite->GetSprite() && m_sprites.front()->GetSprite()->GetTexture() == sprite->GetSprite()->GetTexture()))
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

        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(m_sprites.front()->GetSprite()->GetTexture());
        mat->SetMainColor(m_color);
        if(m_clip)
        {
            Vector3 min = wvp.MultiplyPoint(Vector3(m_clip_rect.x, m_clip_rect.w, 0));
            Vector3 max = wvp.MultiplyPoint(Vector3(m_clip_rect.z, m_clip_rect.y, 0));
            Vector4 rect(min.x, max.y, max.x, min.y);
            min = wvp.MultiplyPoint(Vector3(m_clip_rect.x + m_clip_soft.x, m_clip_rect.w + m_clip_soft.y, 0));
            max = wvp.MultiplyPoint(Vector3(m_clip_rect.z - m_clip_soft.x, m_clip_rect.y - m_clip_soft.y, 0));
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

        DrawIndexed(get_sprites_index_count(m_sprites), 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}

    bool SpriteBatchRenderer::IsPointInClipRect(const Vector3 &point)
    {
        auto rect = GetClipRect();
        auto matrix = GetTransform()->GetLocalToWorldMatrix();
        auto min = matrix.MultiplyPoint3x4(Vector3(rect.x, rect.w));
        auto max = matrix.MultiplyPoint3x4(Vector3(rect.z, rect.y));

        if( point.x < min.x ||
            point.x > max.x ||
            point.y < min.y ||
            point.y > max.y)
        {
            return false;
        }

        return true;
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

        int vertex_count = get_sprites_vertex_count(m_sprites);
        int index_count = get_sprites_index_count(m_sprites);

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
		{
            UpdateVertexBuffer(true);
            UpdateIndexBuffer(true);
		}
		else
		{
            if( m_vertex_count_old != vertex_count ||
                m_index_count_old != index_count)
			{
				Release();

                UpdateVertexBuffer(true);
                UpdateIndexBuffer(true);
			}
			else
			{
				UpdateVertexBuffer(false);
                UpdateIndexBuffer(false);
			}
		}

        m_vertex_count_old = vertex_count;
        m_index_count_old = index_count;
	}

	void SpriteBatchRenderer::Release()
	{
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
	}

	static int fill_vertex_buffer(char *buffer, const std::shared_ptr<SpriteNode> &sprite, SpriteBatchRenderer *batch)
	{
		char *p = buffer;
		auto s = sprite->GetSprite();
        int vertex_count = s->GetVertexCount();
		Vector2 *vertices = s->GetVertices();
		Vector2 *uv = s->GetUV();
		Color c = sprite->GetColor();

        auto local_position = batch->GetTransform()->InverseTransformPoint(sprite->GetTransform()->GetPosition());
        auto local_rotation = Quaternion::Inverse(batch->GetTransform()->GetRotation()) * sprite->GetTransform()->GetRotation();
        const Vector3 &parent_scale = batch->GetTransform()->GetScale();
        Vector3 scale = sprite->GetTransform()->GetScale();
        float x = scale.x / parent_scale.x;
        float y = scale.y / parent_scale.y;
        float z = scale.z / parent_scale.z;
        auto local_scale = Vector3(x, y, z);
        auto matrix = Matrix4x4::TRS(local_position, local_rotation, local_scale);

		for(int i=0; i<vertex_count; i++)
		{
			Vector3 pos = matrix.MultiplyPoint3x4(vertices[i]);
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

	void SpriteBatchRenderer::UpdateVertexBuffer(bool create)
	{
        int vertex_count = get_sprites_vertex_count(m_sprites);
        if(vertex_count)
        {
            int buffer_size = sizeof(VertexUI) * vertex_count;
            char *buffer = (char *) malloc(buffer_size);

            char *p = buffer;
            for(auto &i : m_sprites)
            {
                p += fill_vertex_buffer(p, i, this);
            }

            if(create)
            {
                m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);
            }
            else
            {
                GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);
            }

            free(buffer);
        }
	}

    void SpriteBatchRenderer::UpdateIndexBuffer(bool create)
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
                unsigned short *indices = i->GetSprite()->GetIndices();

                if(i->GetGameObject()->IsActiveInHierarchy() && i->IsEnable())
                {
                    for(int j=0; j<index_count; j++)
                    {
                        unsigned short index = indices[j] + vertex_count;
                        *p = index;
                        p++;
                    }
                }

                vertex_count += i->GetSprite()->GetVertexCount();
            }

            if(create)
            {
                m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Index);
            }
            else
            {
                GraphicsDevice::GetInstance()->UpdateBufferObject(m_index_buffer, buffer, buffer_size);
            }

            free(buffer);
        }
    }
}
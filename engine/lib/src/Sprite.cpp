#include "Sprite.h"
#include "VertexType.h"

namespace Galaxy3D
{
	std::shared_ptr<Sprite> Sprite::LoadFromFile(const std::string &file)
	{
		auto tex = Texture2D::LoadFromFile(file);
		if(tex)
		{
			return Create(tex);
		}
		
		return std::shared_ptr<Sprite>();
	}

	std::shared_ptr<Sprite> Sprite::Create(const std::shared_ptr<Texture2D> &texture)
	{
		int w = texture->GetWidth();
		int h = texture->GetHeight();

		return Create(
			texture,
			Rect(0, 0, (float) w, (float) h),
			Vector2(0.5f, 0.5f),
			100,
			Vector4(0, 0, 0, 0),
            Type::Simple);
	}

	std::shared_ptr<Sprite> Sprite::Create(
			const std::shared_ptr<Texture2D> &texture,
			const Rect &rect,
			const Vector2 &pivot,
			float pixels_per_unit,
            const Vector4 &border,
            Type::Enum type)
	{
		std::shared_ptr<Sprite> sprite(new Sprite());
		sprite->m_texture = texture;
		sprite->m_rect = rect;
		sprite->m_pivot = pivot;
		sprite->m_pixels_per_unit = pixels_per_unit;
		sprite->m_border = border;
        sprite->m_type = type;

        if(sprite->m_type == Type::Simple)
        {
            sprite->FillMeshSimple();
        }
        else if(sprite->m_type == Type::Sliced)
        {
            sprite->FillMeshSliced();
        }

		return sprite;
	}

    Sprite::Sprite()
    {
    }

    Sprite::~Sprite()
    {
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
    }

    void Sprite::FillMeshSimple()
    {
        float v_ppu = 1 / m_pixels_per_unit;

        float v_w = 1.0f / m_texture->GetWidth();
        float v_h = 1.0f / m_texture->GetHeight();

        float left = -m_pivot.x * m_rect.width;
        float top = -m_pivot.y * m_rect.height;

        Rect vertices(left * v_ppu, top * v_ppu, m_rect.width * v_ppu, m_rect.height * v_ppu);
        Rect uv(m_rect.left * v_w, m_rect.top * v_h, m_rect.width * v_w, m_rect.height * v_h);

        m_vertices[0] = Vector2(vertices.left, -vertices.top);
        m_vertices[1] = Vector2(vertices.left, -(vertices.top + vertices.height));
        m_vertices[2] = Vector2(vertices.left + vertices.width, -(vertices.top + vertices.height));
        m_vertices[3] = Vector2(vertices.left + vertices.width, -vertices.top);

        m_uv[0] = Vector2(uv.left, uv.top);
        m_uv[1] = Vector2(uv.left, uv.top + uv.height);
        m_uv[2] = Vector2(uv.left + uv.width, uv.top + uv.height);
        m_uv[3] = Vector2(uv.left + uv.width, uv.top);

        m_triangles[0] = 0;
        m_triangles[1] = 1;
        m_triangles[2] = 2;
        m_triangles[3] = 0;
        m_triangles[4] = 2;
        m_triangles[5] = 3;
    }

    void Sprite::FillMeshSliced()
    {
        if(m_border == Vector4(0, 0, 0, 0))
        {
            FillMeshSimple();
            return;
        }


    }

    static void fill_vertex_buffer(char *buffer, Sprite *sprite)
    {
        char *p = buffer;
        Vector2 *vertices = sprite->GetVertices();
        Vector2 *uv = sprite->GetUV();

        for(int i=0; i<4; i++)
        {
            Vector3 pos = vertices[i];
            memcpy(p, &pos, sizeof(Vector3));
            p += sizeof(Vector3);

            Color c = Color(1, 1, 1, 1);
            memcpy(p, &c, sizeof(Color));
            p += sizeof(Color);

            Vector2 v1 = uv[i];
            memcpy(p, &v1, sizeof(Vector2));
            p += sizeof(Vector2);
        }
    }

    BufferObject Sprite::GetVertexBuffer()
    {
        if(m_vertex_buffer.buffer == NULL)
        {
            int buffer_size = sizeof(VertexUI) * 4;
            char *buffer = (char *) malloc(buffer_size);

            fill_vertex_buffer(buffer, this);

            m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

            free(buffer);
        }

        return m_vertex_buffer;
    }

    BufferObject Sprite::GetIndexBuffer()
    {
        if(m_index_buffer.buffer == NULL)
        {
            unsigned short *uv = this->GetIndices();
            int buffer_size = sizeof(unsigned short) * 6;
            char *buffer = (char *) malloc(buffer_size);
            char *p = buffer;

            memcpy(p, uv, buffer_size);

            m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);

            free(buffer);
        }

        return m_index_buffer;
    }
}
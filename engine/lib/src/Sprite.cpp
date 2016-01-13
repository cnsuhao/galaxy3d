#include "Sprite.h"
#include "VertexType.h"
#include "Texture2D.h"

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

	std::shared_ptr<Sprite> Sprite::Create(const std::shared_ptr<Texture> &texture)
	{
		int w = texture->GetWidth();
		int h = texture->GetHeight();

		return Create(
			texture,
			Rect(0, 0, (float) w, (float) h),
			Vector2(0.5f, 0.5f),
			100,
			Vector4(0, 0, 0, 0),
            Type::Simple,
            Vector2(0, 0));
	}

	std::shared_ptr<Sprite> Sprite::Create(
			const std::shared_ptr<Texture> &texture,
			const Rect &rect,
			const Vector2 &pivot,
			float pixels_per_unit,
            const Vector4 &border,
            Type::Enum type,
            Vector2 size)
	{
		std::shared_ptr<Sprite> sprite(new Sprite());
		sprite->m_texture = texture;
		sprite->m_rect = rect;
		sprite->m_pivot = pivot;
		sprite->m_pixels_per_unit = pixels_per_unit;
		sprite->m_border = border;
        sprite->m_type = type;
        sprite->m_size = size;

        switch(sprite->m_type)
        {
            case Type::Simple:
                sprite->FillMeshSimple();
                break;
            case Type::Sliced:
                sprite->FillMeshSliced();
                break;
            case Type::Tiled:
                sprite->FillMeshTiled();
                break;
            case Type::Filled:
                sprite->FillMeshFilled();
                break;
        }

		return sprite;
	}

    Sprite::Sprite():
        m_fill_amount(1.0f),
        m_fill_direction(FillDirection::Horizontal)
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

        float width, height;
        if(m_size == Vector2(0, 0))
        {
            width = m_rect.width;
            height = m_rect.height;
        }
        else
        {
            width = m_size.x;
            height = m_size.y;
        }

        float v_w = 1.0f / m_texture->GetWidth();
        float v_h = 1.0f / m_texture->GetHeight();

        float left = -m_pivot.x * width;
        float top = -m_pivot.y * height;

        Rect vertices(left * v_ppu, top * v_ppu, width * v_ppu, height * v_ppu);
        Rect uv(m_rect.left * v_w, m_rect.top * v_h, m_rect.width * v_w, m_rect.height * v_h);

        m_vertices.resize(4);
        m_uv.resize(4);
        m_triangles.resize(6);

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

        float v_ppu = 1 / m_pixels_per_unit;

        float width, height;
        if(m_size == Vector2(0, 0))
        {
            width = m_rect.width;
            height = m_rect.height;
        }
        else
        {
            width = m_size.x;
            height = m_size.y;
        }

        float v_w = 1.0f / m_texture->GetWidth();
        float v_h = 1.0f / m_texture->GetHeight();

        m_vertices.resize(16);
        m_uv.resize(16);
        m_triangles.resize(9 * 2 * 3);

        float left = -m_pivot.x * width;
        float top = -m_pivot.y * height;

        Rect vertices(left * v_ppu, top * v_ppu, width * v_ppu, height * v_ppu);
        Rect uv(m_rect.left * v_w, m_rect.top * v_h, m_rect.width * v_w, m_rect.height * v_h);

        Rect vertices_inside(
            (left + m_border.x) * v_ppu,
            (top + m_border.y) * v_ppu,
            (width - m_border.x - m_border.z) * v_ppu,
            (height - m_border.y - m_border.w) * v_ppu);
        Rect uv_inside(
            (m_rect.left + m_border.x) * v_w,
            (m_rect.top + m_border.y) * v_h,
            (m_rect.width - m_border.x - m_border.z) * v_w,
            (m_rect.height - m_border.y - m_border.w) * v_h);

        int i = 0;

        m_vertices[i++] = Vector2(vertices.left, -vertices.top);
        m_vertices[i++] = Vector2(vertices.left, -vertices_inside.top);
        m_vertices[i++] = Vector2(vertices_inside.left, -vertices_inside.top);
        m_vertices[i++] = Vector2(vertices_inside.left, -vertices.top);

        m_vertices[i++] = Vector2(vertices.left, -(vertices_inside.top + vertices_inside.height));
        m_vertices[i++] = Vector2(vertices.left, -(vertices.top + vertices.height));
        m_vertices[i++] = Vector2(vertices_inside.left, -(vertices.top + vertices.height));
        m_vertices[i++] = Vector2(vertices_inside.left, -(vertices_inside.top + vertices_inside.height));

        m_vertices[i++] = Vector2(vertices_inside.left + vertices_inside.width, -(vertices_inside.top + vertices_inside.height));
        m_vertices[i++] = Vector2(vertices_inside.left + vertices_inside.width, -(vertices.top + vertices.height));
        m_vertices[i++] = Vector2(vertices.left + vertices.width, -(vertices.top + vertices.height));
        m_vertices[i++] = Vector2(vertices.left + vertices.width, -(vertices_inside.top + vertices_inside.height));

        m_vertices[i++] = Vector2(vertices_inside.left + vertices_inside.width, -vertices.top);
        m_vertices[i++] = Vector2(vertices_inside.left + vertices_inside.width, -vertices_inside.top);
        m_vertices[i++] = Vector2(vertices.left + vertices.width, -vertices_inside.top);
        m_vertices[i++] = Vector2(vertices.left + vertices.width, -vertices.top);

        i = 0;
        
        m_uv[i++] = Vector2(uv.left, uv.top);
        m_uv[i++] = Vector2(uv.left, uv_inside.top);
        m_uv[i++] = Vector2(uv_inside.left, uv_inside.top);
        m_uv[i++] = Vector2(uv_inside.left, uv.top);

        m_uv[i++] = Vector2(uv.left, uv_inside.top + uv_inside.height);
        m_uv[i++] = Vector2(uv.left, uv.top + uv.height);
        m_uv[i++] = Vector2(uv_inside.left, uv.top + uv.height);
        m_uv[i++] = Vector2(uv_inside.left, uv_inside.top + uv_inside.height);

        m_uv[i++] = Vector2(uv_inside.left + uv_inside.width, uv_inside.top + uv_inside.height);
        m_uv[i++] = Vector2(uv_inside.left + uv_inside.width, uv.top + uv.height);
        m_uv[i++] = Vector2(uv.left + uv.width, uv.top + uv.height);
        m_uv[i++] = Vector2(uv.left + uv.width, uv_inside.top + uv_inside.height);

        m_uv[i++] = Vector2(uv_inside.left + uv_inside.width, uv.top);
        m_uv[i++] = Vector2(uv_inside.left + uv_inside.width, uv_inside.top);
        m_uv[i++] = Vector2(uv.left + uv.width, uv_inside.top);
        m_uv[i++] = Vector2(uv.left + uv.width, uv.top);

        i = 0;

        m_triangles[i++] = 0;
        m_triangles[i++] = 1;
        m_triangles[i++] = 2;
        m_triangles[i++] = 0;
        m_triangles[i++] = 2;
        m_triangles[i++] = 3;

        m_triangles[i++] = 1;
        m_triangles[i++] = 4;
        m_triangles[i++] = 7;
        m_triangles[i++] = 1;
        m_triangles[i++] = 7;
        m_triangles[i++] = 2;

        m_triangles[i++] = 4;
        m_triangles[i++] = 5;
        m_triangles[i++] = 6;
        m_triangles[i++] = 4;
        m_triangles[i++] = 6;
        m_triangles[i++] = 7;

        m_triangles[i++] = 7;
        m_triangles[i++] = 6;
        m_triangles[i++] = 9;
        m_triangles[i++] = 7;
        m_triangles[i++] = 9;
        m_triangles[i++] = 8;

        m_triangles[i++] = 8;
        m_triangles[i++] = 9;
        m_triangles[i++] = 10;
        m_triangles[i++] = 8;
        m_triangles[i++] = 10;
        m_triangles[i++] = 11;

        m_triangles[i++] = 13;
        m_triangles[i++] = 8;
        m_triangles[i++] = 11;
        m_triangles[i++] = 13;
        m_triangles[i++] = 11;
        m_triangles[i++] = 14;

        m_triangles[i++] = 12;
        m_triangles[i++] = 13;
        m_triangles[i++] = 14;
        m_triangles[i++] = 12;
        m_triangles[i++] = 14;
        m_triangles[i++] = 15;

        m_triangles[i++] = 3;
        m_triangles[i++] = 2;
        m_triangles[i++] = 13;
        m_triangles[i++] = 3;
        m_triangles[i++] = 13;
        m_triangles[i++] = 12;

        m_triangles[i++] = 2;
        m_triangles[i++] = 7;
        m_triangles[i++] = 8;
        m_triangles[i++] = 2;
        m_triangles[i++] = 8;
        m_triangles[i++] = 13;
    }

    void Sprite::FillMeshTiled()
    {
        float v_ppu = 1 / m_pixels_per_unit;

        float width, height;
        if(m_size == Vector2(0, 0))
        {
            width = m_rect.width;
            height = m_rect.height;
        }
        else
        {
            width = m_size.x;
            height = m_size.y;
        }

        float v_w = 1.0f / m_texture->GetWidth();
        float v_h = 1.0f / m_texture->GetHeight();

        float left = -m_pivot.x * width;
        float top = -m_pivot.y * height;

        m_vertices.clear();
        m_uv.clear();
        m_triangles.clear();

        float tile_w = m_rect.width - m_border.x - m_border.z;
        float tile_h = m_rect.height - m_border.y - m_border.w;
        int tile_count_x = (int) ceil(width / tile_w);
        int tile_count_y = (int) ceil(height / tile_h);
        Vector2 left_top_uv = Vector2((m_rect.left + m_border.x) * v_w, (m_rect.top + m_border.y) * v_h);
        Vector2 left_bottom_uv = Vector2((m_rect.left + m_border.x) * v_w, (m_rect.top + m_rect.height - m_border.w) * v_h);
        Vector2 right_bottom_uv = Vector2((m_rect.left + m_rect.width - m_border.z) * v_w, (m_rect.top + m_rect.height - m_border.w) * v_h);
        Vector2 right_top_uv = Vector2((m_rect.left + m_rect.width - m_border.z) * v_w, (m_rect.top + m_border.y) * v_h);

        for(int i=0; i<tile_count_y; i++)
        {
            for(int j=0; j<tile_count_x; j++)
            {
                Vector2 min_v(Vector2(left + tile_w * j, top + tile_h * i) * v_ppu);
                Vector2 max_v(Vector2(left + tile_w * (j + 1), top + tile_h * (i + 1)) * v_ppu);
                Vector2 min_uv(left_top_uv);
                Vector2 max_uv(right_bottom_uv);

                if(j == tile_count_x - 1)
                {
                    float w = width - tile_w * (tile_count_x - 1);
                    max_v.x = min_v.x + w * v_ppu;
                    max_uv.x = min_uv.x + w / tile_w;
                }

                if(i == tile_count_y - 1)
                {
                    float h = height - tile_h * (tile_count_y - 1);
                    max_v.y = min_v.y + h * v_ppu;
                    max_uv.y = min_uv.y + h / tile_h;
                }

                m_vertices.push_back(Vector2(min_v.x, -min_v.y));
                m_vertices.push_back(Vector2(min_v.x, -max_v.y));
                m_vertices.push_back(Vector2(max_v.x, -max_v.y));
                m_vertices.push_back(Vector2(max_v.x, -min_v.y));

                m_uv.push_back(min_uv);
                m_uv.push_back(Vector2(min_uv.x, max_uv.y));
                m_uv.push_back(max_uv);
                m_uv.push_back(Vector2(max_uv.x, min_uv.y));

                m_triangles.push_back(tile_count_x * 4 * i + j * 4);
                m_triangles.push_back(tile_count_x * 4 * i + j * 4 + 1);
                m_triangles.push_back(tile_count_x * 4 * i + j * 4 + 2);
                m_triangles.push_back(tile_count_x * 4 * i + j * 4);
                m_triangles.push_back(tile_count_x * 4 * i + j * 4 + 2);
                m_triangles.push_back(tile_count_x * 4 * i + j * 4 + 3);
            }
        }
    }

    void Sprite::FillMeshFilled()
    {
    
    }

    static void fill_vertex_buffer(char *buffer, Sprite *sprite)
    {
        char *p = buffer;
        int vertex_count = sprite->GetVertexCount();
        Vector2 *vertices = sprite->GetVertices();
        Vector2 *uv = sprite->GetUV();

        for(int i=0; i<vertex_count; i++)
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
            int vertex_count = GetVertexCount();
            if(vertex_count > 0)
            {
                int buffer_size = sizeof(VertexUI) * vertex_count;
                char *buffer = (char *) malloc(buffer_size);

                fill_vertex_buffer(buffer, this);

                m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

                free(buffer);
            }
        }

        return m_vertex_buffer;
    }

    BufferObject Sprite::GetIndexBuffer()
    {
        if(m_index_buffer.buffer == NULL)
        {
            int index_count = this->GetIndexCount();
            if(index_count > 0)
            {
                unsigned short *uv = this->GetIndices();
                int buffer_size = sizeof(unsigned short) * index_count;
                char *buffer = (char *) malloc(buffer_size);

                memcpy(buffer, uv, buffer_size);

                m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);

                free(buffer);
            }
        }

        return m_index_buffer;
    }
}
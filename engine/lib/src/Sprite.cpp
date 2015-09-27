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
			Vector4(0, 0, 0, 0));
	}

	std::shared_ptr<Sprite> Sprite::Create(
			const std::shared_ptr<Texture2D> &texture,
			const Rect &rect,
			const Vector2 &pivot,
			float pixels_per_unit,
			const Vector4 &border)
	{
		std::shared_ptr<Sprite> sprite(new Sprite());
		sprite->m_texture = texture;
		sprite->m_rect = rect;
		sprite->m_pivot = pivot;
		sprite->m_pixels_per_unit = pixels_per_unit;
		sprite->m_border = border;

		float v_ppu = 1 / pixels_per_unit;

		float v_w = 1.0f / texture->GetWidth();
		float v_h = 1.0f / texture->GetHeight();

		float left = -pivot.x * rect.width;
		float top = -pivot.y * rect.height;

		Rect vertices(left * v_ppu, top * v_ppu, rect.width * v_ppu, rect.height * v_ppu);
		Rect uv(rect.left * v_w, rect.top * v_h, rect.width * v_w, rect.height * v_h);

		sprite->m_vertices[0] = Vector2(vertices.left, -vertices.top);
		sprite->m_vertices[1] = Vector2(vertices.left, -(vertices.top + vertices.height));
		sprite->m_vertices[2] = Vector2(vertices.left + vertices.width, -(vertices.top + vertices.height));
		sprite->m_vertices[3] = Vector2(vertices.left + vertices.width, -vertices.top);

		sprite->m_uv[0] = Vector2(uv.left, uv.top);
		sprite->m_uv[1] = Vector2(uv.left, uv.top + uv.height);
		sprite->m_uv[2] = Vector2(uv.left + uv.width, uv.top + uv.height);
		sprite->m_uv[3] = Vector2(uv.left + uv.width, uv.top);

		sprite->m_triangles[0] = 0;
		sprite->m_triangles[1] = 1;
		sprite->m_triangles[2] = 2;
		sprite->m_triangles[3] = 0;
		sprite->m_triangles[4] = 2;
		sprite->m_triangles[5] = 3;

		return sprite;
	}

	Sprite::Sprite():
        m_vertex_buffer(NULL),
        m_index_buffer(NULL)
	{
	}

    Sprite::~Sprite()
    {
        SAFE_RELEASE(m_vertex_buffer);
        SAFE_RELEASE(m_index_buffer);
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

    ID3D11Buffer *Sprite::GetVertexBuffer()
    {
        if(m_vertex_buffer == NULL)
        {
            int buffer_size = sizeof(VertexUI) * 4;
            char *buffer = (char *) malloc(buffer_size);

            fill_vertex_buffer(buffer, this);

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

        return m_vertex_buffer;
    }

    ID3D11Buffer *Sprite::GetIndexBuffer()
    {
        if(m_index_buffer == NULL)
        {
            unsigned short *uv = this->GetIndices();
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
            HRESULT hr = device->CreateBuffer(&bd, &data, &m_index_buffer);

            free(buffer);
        }

        return m_index_buffer;
    }
}
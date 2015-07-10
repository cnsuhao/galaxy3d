#include "Sprite.h"

namespace Galaxy3D
{
	std::shared_ptr<Sprite> Sprite::Create(const std::string &file)
	{
		auto tex = Texture2D::LoadImageFile(file);
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

	Sprite::Sprite()
	{
	}
}
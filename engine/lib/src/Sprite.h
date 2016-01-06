#ifndef __Sprite_h__
#define __Sprite_h__

#include "Object.h"
#include "Texture2D.h"
#include "Rect.h"
#include "Vector2.h"
#include "Vector4.h"
#include "GraphicsDevice.h"

namespace Galaxy3D
{
	class Sprite : Object
	{
	public:
		static std::shared_ptr<Sprite> LoadFromFile(const std::string &file);
		static std::shared_ptr<Sprite> Create(const std::shared_ptr<Texture2D> &texture);
		//	rect:指定原贴图上的像素矩形区域，左上为起点
		//	pivot:中心点，相对于sprite rect，(0，0)为左上角，（1，1）为右下角
		//	pixels_per_unit:The number of pixels in the sprite that correspond to one unit in world space. (Read Only)
		//	border:the border sizes of the sprite.
		static std::shared_ptr<Sprite> Create(
			const std::shared_ptr<Texture2D> &texture,
			const Rect &rect,
			const Vector2 &pivot,
			float pixels_per_unit,
			const Vector4 &border);
        ~Sprite();
        BufferObject GetVertexBuffer();
        BufferObject GetIndexBuffer();
        Vector2 *GetVertices() {return m_vertices;}
		Vector2 *GetUV() {return m_uv;}
		unsigned short *GetIndices() {return m_triangles;}
		std::shared_ptr<Texture2D> GetTexture() const {return m_texture;}

	private:
		std::shared_ptr<Texture2D> m_texture;
		Rect m_rect;
		Vector2 m_pivot;
		float m_pixels_per_unit;
		Vector4 m_border;
		Vector2 m_vertices[4];
		Vector2 m_uv[4];
		unsigned short m_triangles[6];
        BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;

		Sprite();
	};
}

#endif
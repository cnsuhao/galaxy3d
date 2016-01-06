#ifndef __TextRenderer_h__
#define __TextRenderer_h__

#include "Renderer.h"
#include "Label.h"

namespace Galaxy3D
{
	class TextRenderer : public Renderer
	{
	public:
		TextRenderer();
		virtual ~TextRenderer();
		void SetLabel(const std::shared_ptr<Label> &label);
		std::shared_ptr<Label> GetLabel() const {return m_label;}
		void UpdateLabel();//设置label后，需要手动调用

	protected:
		virtual void Render(int material_index);

	private:
		std::shared_ptr<Label> m_label;
		BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;
		int m_vertex_count;
		std::vector<BufferObject> m_vertex_buffer_img;
		std::vector<BufferObject> m_index_buffer_img;
		std::shared_ptr<Material> m_material_img;

		void CreateVertexBuffer();
		void UpdateVertexBuffer();
		void CreateIndexBuffer();
		void ReleaseBuffer();
		void RenderLabelImage();
		void CreateVertexBufferLabelImage();
		void UpdateVertexBufferLabelImage();
		void CreateIndexBufferLabelImage();
		void ReleaseBufferLabelImage();
	};
}

#endif
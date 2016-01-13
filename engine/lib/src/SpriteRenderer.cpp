#include "SpriteRenderer.h"

namespace Galaxy3D
{
	SpriteRenderer::SpriteRenderer():
		m_color(1, 1, 1, 1)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

		SetSharedMaterial(Material::Create("UI/Sprite"));
	}

	void SpriteRenderer::SetSprite(const std::shared_ptr<Sprite> &sprite)
	{
        m_sprite = sprite;
	}

	void SpriteRenderer::Render(int material_index)
	{
		if(!m_sprite)
		{
			return;
		}

        auto vertex_buffer = m_sprite->GetVertexBuffer();
        auto index_buffer = m_sprite->GetIndexBuffer();

		if(vertex_buffer.buffer == NULL || index_buffer.buffer == NULL)
		{
			return;
		}

		auto mat = GetSharedMaterial();
		auto shader = mat->GetShader();
		auto pass = shader->GetPass(0);

        GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
        GraphicsDevice::GetInstance()->SetVertexBuffer(vertex_buffer, pass->vs->vertex_stride, 0);
        GraphicsDevice::GetInstance()->SetIndexBuffer(index_buffer, IndexType::UShort);
		
		auto camera = Camera::GetCurrent();
		Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(m_sprite->GetTexture());
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

        DrawIndexed(m_sprite->GetIndexCount(), 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}
}
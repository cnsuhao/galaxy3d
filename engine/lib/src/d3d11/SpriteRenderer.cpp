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

	void SpriteRenderer::Render()
	{
		if(!m_sprite)
		{
			return;
		}

        auto vertex_buffer = m_sprite->GetVertexBuffer();
        auto index_buffer = m_sprite->GetIndexBuffer();

		if(vertex_buffer == NULL || index_buffer == NULL)
		{
			return;
		}

		auto mat = GetSharedMaterial();
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
		auto shader = mat->GetShader();
		auto pass = shader->GetPass(0);

		context->IASetInputLayout(pass->vs->input_layout);

		UINT stride = pass->vs->vertex_stride;
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
		context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
		
		auto camera = Camera::GetCurrent();
		Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(m_sprite->GetTexture());
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

        DrawIndexed(6, 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}
}
#include "SpriteRenderer.h"
#include "Vector4.h"
#include "GameObject.h"
#include "UICanvas.h"
#include "Debug.h"

namespace Galaxy3D
{
	SpriteRenderer::SpriteRenderer():
		m_color(1, 1, 1, 1)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;
		
        auto mat = Material::Create("UI/Sprite");
		SetSharedMaterial(mat);
	}

    void SpriteRenderer::Start()
    {
        auto canvas = GetGameObject()->GetComponentInParent<UICanvas>();

        if(canvas && m_anchor)
        {
			auto t = GetTransform();
            canvas->AnchorTransform(t, *m_anchor);
        }
    }

    void SpriteRenderer::SetAnchor(const Vector4 &anchor)
    {
        m_anchor = std::make_shared<Vector4>(anchor);
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
        GraphicsDevice::GetInstance()->SetVertexBuffer(vertex_buffer, pass->vs);
        GraphicsDevice::GetInstance()->SetIndexBuffer(index_buffer, IndexType::UShort);
		
		auto camera = Camera::GetCurrent();
		Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(m_sprite->GetTexture());
        mat->SetMainColor(m_color);
		mat->SetVector("ClipRect", Vector4(-1, 1, 1, -1));
		mat->SetVector("ClipSoft", Vector4(-1, 1, 1, -1));
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

        DrawIndexed(m_sprite->GetIndexCount(), 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}
}
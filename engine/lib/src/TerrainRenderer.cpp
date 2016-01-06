#include "TerrainRenderer.h"
#include "GameObject.h"

namespace Galaxy3D
{
    void TerrainRenderer::Start()
    {
        auto terrain = GetGameObject()->GetComponent<Terrain>();
        if(terrain)
        {
            m_terrain = terrain;
            SetSharedMaterial(m_terrain->GetSharedMaterial());
        }
    }

    void TerrainRenderer::Render(int material_index)
    {
        if(!m_terrain)
        {
            return;
        }

        auto vertex_buffer = m_terrain->GetVertexBuffer();
        auto index_buffer = m_terrain->GetIndexBuffer();

        if(vertex_buffer.buffer == NULL || index_buffer.buffer == NULL)
        {
            return;
        }

        auto camera = Camera::GetCurrent();
        auto mat = GetSharedMaterial();
        auto shader = mat->GetShader();
        auto pass = shader->GetPass(0);

        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();
        mat->SetMatrix("WorldViewProjection", wvp);

        GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
        GraphicsDevice::GetInstance()->SetVertexBuffer(vertex_buffer, pass->vs->vertex_stride, 0);
        GraphicsDevice::GetInstance()->SetIndexBuffer(index_buffer, IndexType::UInt);

        mat->ReadyPass(0);
        pass->rs->Apply();
        mat->ApplyPass(0);

        DrawIndexed(m_terrain->GetIndexCount(), 0);

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }
}
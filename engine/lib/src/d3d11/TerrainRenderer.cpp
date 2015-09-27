#include "TerrainRenderer.h"

namespace Galaxy3D
{
    void TerrainRenderer::Render()
    {
        if(!m_terrain)
        {
            return;
        }

        auto vertex_buffer = m_terrain->GetVertexBuffer();
        auto index_buffer = m_terrain->GetIndexBuffer();

        if(vertex_buffer == NULL || index_buffer == NULL)
        {
            return;
        }

        auto camera = Camera::GetCurrent();
        auto mat = GetSharedMaterial();
        auto shader = mat->GetShader();
        auto pass = shader->GetPass(0);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();
        mat->SetMatrix("WorldViewProjection", wvp);

        context->IASetInputLayout(pass->vs->input_layout);
        UINT stride = pass->vs->vertex_stride;
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

        mat->ReadyPass(0);
        pass->rs->Apply();
        mat->ApplyPass(0);

        DrawIndexed(m_terrain->GetIndexCount(), 0);

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }
}
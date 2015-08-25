#include "SkinnedMeshRenderer.h"

namespace Galaxy3D
{
    void SkinnedMeshRenderer::Render()
    {
        if(!m_mesh)
        {
            return;
        }

        auto vertex_buffer = m_mesh->GetVertexBuffer();
        auto index_buffer = m_mesh->GetIndexBuffer();

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto camera = Camera::GetCurrent();

        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

        auto mats = GetSharedMaterials();
        for(size_t i=0; i<mats.size(); i++)
        {
            auto mat = mats[i];
            auto shader = mat->GetShader();

            mat->SetMatrix("WorldViewProjection", wvp);

            int offset = 0;
            int index_count = m_mesh->GetIndexCount(i);

            auto pass_count = shader->GetPassCount();
            for(int j=0; j<pass_count; j++)
            {
                auto pass = shader->GetPass(j);

                if(i == 0 && j == 0)
                {
                    context->IASetInputLayout(pass->vs->input_layout);
                    UINT stride = pass->vs->vertex_stride;
                    UINT offset = 0;
                    context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
                    context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
                }

                mat->ReadyPass(j);
                pass->rs->Apply();
                mat->ApplyPass(j);

                DrawIndexed(index_count, offset);
            }

            offset += index_count;
        }

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }
}
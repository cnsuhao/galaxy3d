#include "ParticleSystemRenderer.h"
#include "ParticleSystem.h"

namespace Galaxy3D
{
    void ParticleSystemRenderer::Render(int material_index)
    {
        auto ps = particle_system.lock();

        auto vertex_buffer = ps->GetVertexBuffer();
        auto index_buffer = ps->GetIndexBuffer();

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto camera = Camera::GetCurrent();

        Matrix4x4 mat_world = Matrix4x4::TRS(GetTransform()->GetPosition(), GetTransform()->GetRotation(), Vector3(1, 1, 1));
        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * mat_world;

        int index_count = ps->GetParticleCount() * 6;
        if(index_count > 0)
        {
            auto mat = GetSharedMaterial();
            auto shader = mat->GetShader();

            mat->SetMatrix("WorldViewProjection", wvp);

            auto pass_count = shader->GetPassCount();
            for(int j=0; j<pass_count; j++)
            {
                auto pass = shader->GetPass(j);

                if(j == 0)
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

                DrawIndexed(index_count, 0);
            }

            GraphicsDevice::GetInstance()->ClearShaderResources();
        }
    }
}
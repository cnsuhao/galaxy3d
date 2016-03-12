#include "ParticleSystemRenderer.h"
#include "ParticleSystem.h"

namespace Galaxy3D
{
    void ParticleSystemRenderer::Render(int material_index)
    {
        auto ps = particle_system.lock();

        auto vertex_buffer = ps->GetVertexBuffer();
        auto index_buffer = ps->GetIndexBuffer();
        auto camera = Camera::GetCurrent();

        Matrix4x4 local_to_world = Matrix4x4::TRS(GetTransform()->GetPosition(), GetTransform()->GetRotation(), Vector3(1, 1, 1));
        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * local_to_world;

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
                    GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
                    GraphicsDevice::GetInstance()->SetVertexBuffer(vertex_buffer, pass->vs);
                    GraphicsDevice::GetInstance()->SetIndexBuffer(index_buffer, IndexType::UShort);
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
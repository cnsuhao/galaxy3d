#include "MeshRenderer.h"
#include "RenderSettings.h"

namespace Galaxy3D
{
    void MeshRenderer::Render(int material_index)
    {
        if(!m_mesh)
        {
            return;
        }

        auto vertex_buffer = m_mesh->GetVertexBuffer();
        auto index_buffer = m_mesh->GetIndexBuffer();

        if(vertex_buffer == NULL || index_buffer == NULL)
        {
            return;
        }

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto camera = Camera::GetCurrent();

        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

        int index_offset = 0;
        for(int i=0; i<material_index; i++)
        {
            index_offset += m_mesh->GetIndexCount(i);
        }

        auto mats = GetSharedMaterials();
        do
        {
            int index_count = m_mesh->GetIndexCount(material_index);
            auto mat = mats[material_index];

            if(!mat)
            {
                continue;
            }

            auto shader = mat->GetShader();

            mat->SetMatrix("WorldViewProjection", wvp);
            mat->SetMatrix("World", GetTransform()->GetLocalToWorldMatrix());
            mat->SetVector("EyePosition", Vector4(camera->GetTransform()->GetPosition()));
            mat->SetColor("GlobalAmbient", RenderSettings::light_ambient);
            mat->SetVector("LightDirection", Vector4(RenderSettings::light_directional_direction));
            mat->SetColor("LightColor", RenderSettings::light_directional_color * RenderSettings::light_directional_intensity);

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

                DrawIndexed(index_count, index_offset);
            }
        }while(false);

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }
}
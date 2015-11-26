#include "SkinnedMeshRenderer.h"
#include "RenderSettings.h"

namespace Galaxy3D
{
    void SkinnedMeshRenderer::Render(int material_index)
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

        //bones
        auto &bindposes = m_mesh->GetBindPoses();
        std::vector<Vector4> bone_matrix(m_bones.size() * 3);
        for(size_t i=0; i<m_bones.size(); i++)
        {
            auto mat = m_bones[i]->GetLocalToWorldMatrix() * bindposes[i];
            bone_matrix[i * 3 + 0] = mat.GetRow(0);
            bone_matrix[i * 3 + 1] = mat.GetRow(1);
            bone_matrix[i * 3 + 2] = mat.GetRow(2);
        }

        Matrix4x4 view_projection = camera->GetViewProjectionMatrix();
        
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
            auto shader = mat->GetShader();

            mat->SetMatrix("ViewProjection", view_projection);
            mat->SetVectorArray("Bones", bone_matrix);
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
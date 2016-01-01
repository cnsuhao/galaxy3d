#include "SkinnedMeshRenderer.h"
#include "RenderSettings.h"

namespace Galaxy3D
{
    DEFINE_COM_CLASS(SkinnedMeshRenderer);

    void SkinnedMeshRenderer::DeepCopy(std::shared_ptr<Object> &source)
    {
        auto src_renderer = std::dynamic_pointer_cast<SkinnedMeshRenderer>(source);

        Renderer::DeepCopy(source);

        m_mesh = src_renderer->m_mesh;
        m_bones = src_renderer->m_bones;
    }

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
            mat->SetVector("LightDirection", Vector4(RenderSettings::GetGlobalDirectionalLight()->GetTransform()->GetRotation() * Vector3(0, 0, 1)));
            mat->SetColor("LightColor", RenderSettings::GetGlobalDirectionalLight()->GetColor() * RenderSettings::GetGlobalDirectionalLight()->GetIntensity());

            auto shadow_light = RenderSettings::GetLightRenderingShadowMap();

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

                bool right_pass = false;
                if(shadow_light)
                {
                    if(pass->name == "depth")
                    {
                        view_projection = shadow_light->GetViewProjectionMatrix();
                        mat->SetMatrix("ViewProjection", view_projection);
                        right_pass = true;
                    }
                }
                else
                {
                    if(camera->IsDeferredShading())
                    {
                        if(pass->name == "deferred")
                        {
                            right_pass = true;
                        }
                    }
                    else
                    {
                        if( pass->name != "depth" &&
                            pass->name != "deferred")
                        {
                            right_pass = true;
                        }
                    }
                }

                if(right_pass)
                {
                    mat->ReadyPass(j);
                    pass->rs->Apply();
                    mat->ApplyPass(j);

                    DrawIndexed(index_count, index_offset);
                }
            }
        }while(false);

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }
}
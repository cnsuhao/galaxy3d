#include "MeshRenderer.h"
#include "RenderSettings.h"
#include "LightmapSettings.h"

namespace Galaxy3D
{
    DEFINE_COM_CLASS(MeshRenderer);

    void MeshRenderer::DeepCopy(std::shared_ptr<Object> &source)
    {
        auto src_renderer = std::dynamic_pointer_cast<MeshRenderer>(source);

        Renderer::DeepCopy(source);

        m_mesh = src_renderer->m_mesh;
    }

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

            auto shader = mat->GetShader();

            mat->SetMatrix("WorldViewProjection", wvp);
            mat->SetMatrix("World", GetTransform()->GetLocalToWorldMatrix());
            mat->SetVector("EyePosition", Vector4(camera->GetTransform()->GetPosition()));
            mat->SetColor("GlobalAmbient", RenderSettings::light_ambient);
            mat->SetVector("LightDirection", Vector4(RenderSettings::light_directional_rotation * Vector3(0, 0, 1)));
            mat->SetColor("LightColor", RenderSettings::light_directional_color * RenderSettings::light_directional_intensity);

            if(m_lightmap_index >= 0)
            {
                mat->SetVector("_LightmapST", m_lightmap_tiling_offset);
                mat->SetTexture("_Lightmap", LightmapSettings::lightmaps[m_lightmap_index]);
            }

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

                if(camera->IsDeferredShading() == (pass->name == "deferred"))
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

    void MeshRenderer::RenderStaticBatch(const RenderBatch *batch, const RenderBatch *last_batch)
    {
        auto mat = batch->renderer->GetSharedMaterials()[batch->material_index];
        auto shader = mat->GetShader();
        auto pass = shader->GetPass(0);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        auto camera = Camera::GetCurrent();
        Matrix4x4 wvp = camera->GetViewProjectionMatrix();

        bool set_buffer = false;
        bool set_material = false;
        bool set_lightmap = false;

        if(last_batch == NULL)
        {
            set_buffer = true;
            set_material = true;
            set_lightmap = true;
        }
        else
        {
            if(!last_batch->IsStaticSinglePassMeshRenderer())
            {
                set_buffer = true;
                set_material = true;
                set_lightmap = true;
            }
            else
            {
                auto last_mat = last_batch->renderer->GetSharedMaterials()[last_batch->material_index];

                if(last_mat->GetGuid() != mat->GetGuid())
                {
                    set_material = true;
                    set_lightmap = true;
                }
                else
                {
                    if(last_batch->renderer->GetLightmapIndex() != batch->renderer->GetLightmapIndex())
                    {
                        set_lightmap = true;
                    }
                }
            }
        }
        
        if(set_buffer)
        {
            UINT stride = pass->vs->vertex_stride;
            UINT offset = 0;
            context->IASetInputLayout(pass->vs->input_layout);
            context->IASetVertexBuffers(0, 1, &m_static_batching_vertex_buffer, &stride, &offset);
            context->IASetIndexBuffer(m_static_batching_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        }

        if(set_material)
        {
            mat->SetMatrix("WorldViewProjection", wvp);
            mat->SetMatrix("World", Matrix4x4::Identity());
            mat->SetVector("EyePosition", Vector4(camera->GetTransform()->GetPosition()));
            mat->SetColor("GlobalAmbient", RenderSettings::light_ambient);
            mat->SetVector("LightDirection", Vector4(RenderSettings::light_directional_rotation * Vector3(0, 0, 1)));
            mat->SetColor("LightColor", RenderSettings::light_directional_color * RenderSettings::light_directional_intensity);
        }

        if(set_lightmap)
        {
            mat->SetTextureDirectlyPS("_Lightmap", LightmapSettings::lightmaps[m_lightmap_index], 0);
        }

        if(m_lightmap_index >= 0)
        {
            mat->SetVectorDirectlyVS("_LightmapST", m_lightmap_tiling_offset, 0);
        }

        if(set_material)
        {
            mat->ReadyPass(0);
            pass->rs->Apply();
            mat->ApplyPass(0);
        }

        DrawIndexed(batch->static_batching_index_count, batch->static_batching_index_offset);
    }
}
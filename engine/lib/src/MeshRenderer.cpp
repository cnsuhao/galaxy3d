#include "MeshRenderer.h"
#include "RenderSettings.h"
#include "LightmapSettings.h"

namespace Galaxy3D
{
    DEFINE_COM_CLASS(MeshRenderer);

    void MeshRenderer::CalculateBounds()
    {
        auto &vertices = m_mesh->GetVertices();
        auto tran = GetTransform();

        Vector3 box_max = Vector3(1, 1, 1) * Mathf::MinFloatValue;
        Vector3 box_min = Vector3(1, 1, 1) * Mathf::MaxFloatValue;
        for(size_t j=0; j<vertices.size(); j++)
        {
            auto &v = vertices[j].POSITION;
            auto v_world = tran->TransformPoint(v);

            box_max = Vector3::Max(box_max, v_world);
            box_min = Vector3::Min(box_min, v_world);
        }
        Bounds bounds((box_max + box_min) * 0.5f, (box_max - box_min) * 0.5f);

        SetBounds(bounds);
    }

    void MeshRenderer::DeepCopy(const std::shared_ptr<Object> &source)
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

        if(vertex_buffer.buffer == NULL || index_buffer.buffer == NULL)
        {
            return;
        }

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
            mat->SetVector("LightDirection", Vector4(RenderSettings::GetGlobalDirectionalLight()->GetTransform()->GetRotation() * Vector3(0, 0, 1)));
            mat->SetColor("LightColor", RenderSettings::GetGlobalDirectionalLight()->GetColor() * RenderSettings::GetGlobalDirectionalLight()->GetIntensity());

            if(m_lightmap_index >= 0)
            {
                mat->SetVector("_LightmapST", m_lightmap_tiling_offset);
                mat->SetTexture("_Lightmap", LightmapSettings::lightmaps[m_lightmap_index]);
            }

            auto shadow_light = RenderSettings::GetLightRenderingShadowMap();

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

                bool right_pass = false;
                if(shadow_light)
                {
                    if(pass->name == "depth")
                    {
                        wvp = shadow_light->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();
                        mat->SetMatrix("WorldViewProjection", wvp);
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

							RenderSettings::GetGlobalDirectionalLight()->SetMaterialShadowParams(mat);
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

    void MeshRenderer::RenderStaticBatch(const RenderBatch *batch, const RenderBatch *last_batch)
    {
        auto mat = batch->renderer->GetSharedMaterials()[batch->material_index];
        auto shader = mat->GetShader();
        auto camera = Camera::GetCurrent();
        Matrix4x4 wvp = camera->GetViewProjectionMatrix();
        int pass_index = 0;

        auto shadow_light = RenderSettings::GetLightRenderingShadowMap();
        auto pass_count = shader->GetPassCount();
        for(int i=0; i<pass_count; i++)
        {
            auto pass = shader->GetPass(i);
            if(shadow_light)
            {
                if(pass->name == "depth")
                {
                    wvp = shadow_light->GetViewProjectionMatrix();
                    pass_index = i;
                    break;
                }
            }
            else
            {
                if(camera->IsDeferredShading())
                {
                    if(pass->name == "deferred")
                    {
                        pass_index = i;
                        break;
                    }
                }
                else
                {
                    if( pass->name != "depth" &&
                        pass->name != "deferred")
                    {
                        pass_index = i;
                        break;
                    }
                }
            }
        }

        auto pass = shader->GetPass(pass_index);

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
            GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
            GraphicsDevice::GetInstance()->SetVertexBuffer(m_static_batching_vertex_buffer, pass->vs);
            GraphicsDevice::GetInstance()->SetIndexBuffer(m_static_batching_index_buffer, IndexType::UInt);
        }

        if(set_material)
        {
            mat->SetMatrix("WorldViewProjection", wvp);
            mat->SetMatrix("World", Matrix4x4::Identity());
            mat->SetVector("EyePosition", Vector4(camera->GetTransform()->GetPosition()));
            mat->SetColor("GlobalAmbient", RenderSettings::light_ambient);
            mat->SetVector("LightDirection", Vector4(RenderSettings::GetGlobalDirectionalLight()->GetTransform()->GetRotation() * Vector3(0, 0, 1)));
            mat->SetColor("LightColor", RenderSettings::GetGlobalDirectionalLight()->GetColor() * RenderSettings::GetGlobalDirectionalLight()->GetIntensity());
        }

        if(set_lightmap)
        {
            if(m_lightmap_index >= 0 && m_lightmap_index < (int) LightmapSettings::lightmaps.size())
            {
                mat->SetTextureDirectlyPS("_Lightmap", LightmapSettings::lightmaps[m_lightmap_index], pass_index);
            }
        }

        if(m_lightmap_index >= 0)
        {
            mat->SetVectorDirectlyVS("_LightmapST", m_lightmap_tiling_offset, pass_index);
        }

        if(set_material)
        {
            mat->ReadyPass(pass_index);
            pass->rs->Apply();
            mat->ApplyPass(pass_index);
        }

        DrawIndexed(batch->static_batching_index_count, batch->static_batching_index_offset);
    }
}
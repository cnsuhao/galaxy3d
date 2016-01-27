#include "TerrainRenderer.h"
#include "GameObject.h"
#include "RenderSettings.h"
#include "LightmapSettings.h"

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

        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();
        mat->SetMatrix("WorldViewProjection", wvp);
        mat->SetMatrix("World", GetTransform()->GetLocalToWorldMatrix());
        mat->SetVector("EyePosition", Vector4(camera->GetTransform()->GetPosition()));
        mat->SetColor("GlobalAmbient", RenderSettings::light_ambient);
        mat->SetVector("LightDirection", Vector4(RenderSettings::GetGlobalDirectionalLight()->GetTransform()->GetRotation() * Vector3(0, 0, 1)));
        mat->SetColor("LightColor", RenderSettings::GetGlobalDirectionalLight()->GetColor() * RenderSettings::GetGlobalDirectionalLight()->GetIntensity());

        int pass_index = 0;
        auto pass_count = shader->GetPassCount();
        for(int j=0; j<pass_count; j++)
        {
            auto pass = shader->GetPass(j);

            if(camera->IsDeferredShading())
            {
                if(pass->name == "deferred")
                {
                    pass_index = j;
                    break;
                }
            }
            else
            {
                pass_index = j;
                break;
            }
        }

        auto pass = shader->GetPass(pass_index);
        GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
        GraphicsDevice::GetInstance()->SetVertexBuffer(vertex_buffer, pass->vs->vertex_stride, 0);
        GraphicsDevice::GetInstance()->SetIndexBuffer(index_buffer, IndexType::UInt);

        mat->ReadyPass(pass_index);
        pass->rs->Apply();
        mat->ApplyPass(pass_index);

        DrawIndexed(m_terrain->GetIndexCount(), 0);

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }
}
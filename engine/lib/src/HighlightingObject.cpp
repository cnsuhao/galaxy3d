#include "HighlightingObject.h"
#include "GameObject.h"
#include "SkinnedMeshRenderer.h"

namespace Galaxy3D
{
    void HighlightingObject::Start()
    {
        auto renderer = GetGameObject()->GetComponent<Renderer>();

        auto skinned_mesh = std::dynamic_pointer_cast<SkinnedMeshRenderer>(renderer);
        if(skinned_mesh)
        {
            m_shader_highlighting = Shader::Find("SkinnedMesh/Highlighting");

            m_materials = renderer->GetSharedMaterials();
            m_shaders_default.resize(m_materials.size());
            for(size_t i=0; i<m_shaders_default.size(); i++)
            {
                m_shaders_default[i] = m_materials[i]->GetShader();
            }
        }
    }

    void HighlightingObject::OnWillRenderObject(int material_index)
    {
        auto camera = Camera::GetCurrent();

        if(camera->GetName() == "camera_highlighting")
        {
            m_materials[material_index]->SetShader(m_shader_highlighting);
            m_materials[material_index]->SetColor("HighlightingColor", m_color);
        }
        else
        {
            m_materials[material_index]->SetShader(m_shaders_default[material_index]);
        }
    }
}
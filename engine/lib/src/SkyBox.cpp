#include "SkyBox.h"

namespace Galaxy3D
{
    void SkyBox::Start()
    {
        m_cube = Mesh::CreateMeshCube();
        m_material = Material::Create("SkyBox");
        m_material->SetMainTexture(m_cubmap);
    }

    void SkyBox::Render()
    {

    }
}
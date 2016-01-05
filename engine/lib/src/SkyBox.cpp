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
        auto camera = Camera::GetCurrent();
        auto world = Matrix4x4::TRS(camera->GetTransform()->GetPosition(), Quaternion::Identity(), Vector3(1, 1, 1) * camera->GetClipNear() * 10);
        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * world;

        m_material->SetMatrix("WorldViewProjection", wvp);
        GraphicsDevice::GetInstance()->DrawMeshNow(m_cube, 0, m_material, 0);
    }
}
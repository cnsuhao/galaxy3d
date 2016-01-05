#ifndef __SkyBox_h__
#define __SkyBox_h__

#include "Component.h"
#include "Cubemap.h"
#include "Mesh.h"

namespace Galaxy3D
{
    class SkyBox : public Component
    {
    public:
        void SetCubemap(const std::shared_ptr<Cubemap> &map) {m_cubmap = map;}
        virtual void Start();
        void Render();

    private:
        std::shared_ptr<Cubemap> m_cubmap;
        std::shared_ptr<Mesh> m_cube;
        std::shared_ptr<Material> m_material;
    };
}

#endif
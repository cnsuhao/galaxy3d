#ifndef __SkyBox_h__
#define __SkyBox_h__

#include "Component.h"
#include "Cubemap.h"

namespace Galaxy3D
{
    class SkyBox : public Component
    {
    public:
        void SetCubemap(const std::shared_ptr<Cubemap> &map) {m_cubmap = map;}

    private:
        std::shared_ptr<Cubemap> m_cubmap;
    };
}

#endif
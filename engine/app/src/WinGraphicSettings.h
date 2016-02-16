#ifndef __WinGraphicSettings_h__
#define __WinGraphicSettings_h__

#include "Component.h"

namespace Galaxy3D
{
    class Camera;

    class WinGraphicSettings : public Component
    {
    public:
        std::shared_ptr<Camera> cam3d;
        
        void Init();
    };
}

#endif
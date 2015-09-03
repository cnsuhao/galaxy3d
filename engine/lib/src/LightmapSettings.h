#ifndef __LightmapSettings_h__
#define __LightmapSettings_h__

#include "Texture2D.h"

namespace Galaxy3D
{
    class LightmapSettings
    {
    public:
        static std::vector<std::shared_ptr<Texture2D>> lightmaps;
    };
}

#endif
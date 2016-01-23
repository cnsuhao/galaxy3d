#ifndef __UIAtlas_h__
#define __UIAtlas_h__

#include "Component.h"

namespace Galaxy3D
{
    class UIAtlas : public Component
    {
    public:
        static std::shared_ptr<UIAtlas> LoadJsonFile(const std::string &file);
    };
}

#endif
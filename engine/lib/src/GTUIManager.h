#ifndef __GTUIManager_h__
#define __GTUIManager_h__

#include "GameObject.h"
#include "UIAtlas.h"
#include <unordered_map>

namespace Galaxy3D
{
    class GTUIManager
    {
    public:
        static void Done();
        static void LoadFont(const std::string &name, const std::string &file);
        static std::shared_ptr<UIAtlas> LoadAtlas(const std::string &name, const std::string &file);
        static std::shared_ptr<GameObject> LoadWindowFromJsonFile(const std::string &file, int sorting_layer, float pixel_per_unit);
        static std::shared_ptr<UIAtlas> FindAtlas(const std::string &name);

    private:
        static std::unordered_map<std::string, std::shared_ptr<UIAtlas>> m_atlases;
    };
}

#endif
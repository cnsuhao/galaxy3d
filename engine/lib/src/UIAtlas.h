#ifndef __UIAtlas_h__
#define __UIAtlas_h__

#include "Component.h"
#include "Rect.h"
#include "Texture2D.h"
#include "Sprite.h"
#include <unordered_map>

namespace Galaxy3D
{
    class UIAtlas : public Component
    {
    public:
        struct SpriteData
        {
            std::string name;
            Rect rect;
            Vector4 border;
        };

        static std::shared_ptr<UIAtlas> LoadFromJsonFile(const std::string &file);
        std::shared_ptr<Sprite> CreateSprite(
            const std::string &name,
            const Vector2 &pivot,
            float pixels_per_unit,
            Sprite::Type::Enum type,
            const Vector2 &size);
        void SetSpriteData(std::shared_ptr<Sprite> &sprite, const std::string &name);

    private:
        std::unordered_map<std::string, SpriteData> m_sprites;
        std::shared_ptr<Texture2D> m_texture;
    };
}

#endif
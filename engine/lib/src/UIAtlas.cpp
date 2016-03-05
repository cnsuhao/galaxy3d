#include "UIAtlas.h"
#include "GTFile.h"
#include "GameObject.h"
#include "json.h"

namespace Galaxy3D
{
    std::shared_ptr<UIAtlas> UIAtlas::LoadFromJsonFile(const std::string &file)
    {
        std::shared_ptr<UIAtlas> atlas;

        if(GTFile::Exist(file))
        {
            std::string str;
            GTFile::ReadAllText(file, str);

            if(!str.empty())
            {
                Json::Reader reader;
                Json::Value root;

                if(reader.parse(str, root))
                {
                    auto frames = root["frames"];
                    auto meta = root["meta"];
                    auto image = meta["image"].asString();

                    atlas = GameObject::Create(image)->AddComponent<UIAtlas>();
                    atlas->m_texture = Texture2D::LoadFromFile(file.substr(0, file.find_last_of('/') + 1) + image);

                    for(size_t i=0; i<frames.size(); i++)
                    {
                        auto sprite = frames[i];

                        auto filename = sprite["filename"].asString();
                        auto frame = sprite["frame"];
                        auto x = frame["x"].asInt();
                        auto y = frame["y"].asInt();
                        auto w = frame["w"].asInt();
                        auto h = frame["h"].asInt();
                        auto border = sprite["border"];
                        auto bx = border["x"].asInt();
                        auto by = border["y"].asInt();
                        auto bz = border["z"].asInt();
                        auto bw = border["w"].asInt();

                        auto find = filename.find_last_of('.');
                        if(find != std::string::npos)
                        {
                            filename = filename.substr(0, find);
                        }

                        SpriteData data;
                        data.name = filename;
                        data.rect = Rect((float) x, (float) y, (float) w, (float) h);
                        data.border = Vector4((float) bx, (float) by, (float) bz, (float) bw);

                        atlas->m_sprites[filename] = data;
                    }
                }
            }
        }

        return atlas;
    }

    std::shared_ptr<Sprite> UIAtlas::CreateSprite(
        const std::string &name,
        const Vector2 &pivot,
        float pixels_per_unit,
        Sprite::Type::Enum type,
        const Vector2 &size,
		const Vector4 &rect_offset)
    {
        std::shared_ptr<Sprite> sprite;

        auto find = m_sprites.find(name);
        if(find == m_sprites.end())
        {
            return sprite;
        }

		Rect rect = find->second.rect;
		rect.left += rect_offset.x;
		rect.top += rect_offset.y;
		rect.width = rect.width - rect_offset.x + rect_offset.z;
		rect.height = rect.height - rect_offset.y + rect_offset.w;

        sprite = Sprite::Create(
            m_texture,
            rect,
            pivot,
            pixels_per_unit,
            find->second.border,
            type,
            size);
        sprite->SetAtlas(std::dynamic_pointer_cast<UIAtlas>(GetComponentPtr()));
		sprite->SetName(name);

        return sprite;
    }

    void UIAtlas::SetSpriteData(std::shared_ptr<Sprite> &sprite, const std::string &name)
    {
        if(sprite->GetTexture() != m_texture)
        {
            return;
        }

        auto find = m_sprites.find(name);
        if(find == m_sprites.end())
        {
            return;
        }

        sprite->SetSpriteData(find->second.rect, find->second.border);
    }
}
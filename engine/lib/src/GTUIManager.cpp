#include "GTUIManager.h"
#include "GTFile.h"
#include "SpriteBatchRenderer.h"
#include "TextRenderer.h"
#include "Layer.h"
#include "BoxCollider.h"
#include "Debug.h"
#include "json.h"

namespace Galaxy3D
{
    std::unordered_map<std::string, std::shared_ptr<UIAtlas>> GTUIManager::m_atlases;

    void GTUIManager::Done()
    {
        m_atlases.clear();
    }

    std::shared_ptr<UIAtlas> GTUIManager::LoadAtlas(const std::string &name, const std::string &file)
    {
        auto atlas = UIAtlas::LoadFromJsonFile(file);
        if(atlas)
        {
            m_atlases[name] = atlas;
        }

        return atlas;
    }

    std::shared_ptr<UIAtlas> GTUIManager::FindAtlas(const std::string &name)
    {
        auto find = m_atlases.find(name);
        if(find != m_atlases.end())
        {
            return find->second;
        }

        return std::shared_ptr<UIAtlas>();
    }

    static Vector3 read_vector3(const Json::Value &node, const std::string &key)
    {
        auto v = node[key];
        float x = (float) v["x"].asDouble();
        float y = (float) v["y"].asDouble();
        float z = (float) v["z"].asDouble();

        return Vector3(x, y, z);
    }

    static Color read_color(const Json::Value &node, const std::string &key)
    {
        auto c = node[key];
        float r = (float) c["r"].asDouble();
        float g = (float) c["g"].asDouble();
        float b = (float) c["b"].asDouble();
        float a = (float) c["a"].asDouble();

        return Color(r, g, b, a);
    }

    static std::shared_ptr<GameObject> read_window_json_node(
        const Json::Value &node,
        std::shared_ptr<Transform> &parent,
        int sorting_layer,
        std::shared_ptr<SpriteBatchRenderer> &batch,
        float pixel_per_unit)
    {
        std::shared_ptr<GameObject> obj;

        auto name = node["name"].asString();
        auto local_position = read_vector3(node, "local_position");
        auto rot = node["local_rotation"];
        float x = (float) rot["x"].asDouble();
        float y = (float) rot["y"].asDouble();
        float z = (float) rot["z"].asDouble();
        float w = (float) rot["w"].asDouble();
        auto local_rotation = Quaternion(x, y, z, w);
        auto local_scale = read_vector3(node, "local_scale");
        auto active = node["active"].asBool();

        obj = GameObject::Create(name);
        obj->SetLayer(Layer::UI);
        obj->SetActive(active);

        if(!batch)
        {
            batch = obj->AddComponent<SpriteBatchRenderer>();
            batch->SetSortingOrder(sorting_layer, 0);
        }

        auto transform = obj->GetTransform();
        transform->SetParent(parent);
        transform->SetLocalPosition(local_position);
        transform->SetLocalRotation(local_rotation);
        transform->SetLocalScale(local_scale);

        auto components = node["components"];
        for(size_t i=0; i<components.size(); i++)
        {
            auto com = components[i];
            auto type = com["type"].asString();

            if(type == "Sprite")
            {
                auto sprite_name = com["sprite"].asString();
                auto sprite_atlas = com["sprite_atlas"].asString();
                auto sprite_type = com["sprite_type"].asInt();
                auto sprite_flip = com["sprite_flip"].asInt();
                auto sprite_fill_dir = com["sprite_fill_dir"].asInt();
                auto sprite_fill_amount = (float) com["sprite_fill_amount"].asDouble();
                auto sprite_fill_invert = com["sprite_fill_invert"].asBool();
                auto color = read_color(com, "color");
                auto pivot = com["pivot"].asInt();
                auto depth = com["depth"].asInt();
                auto width = com["width"].asInt();
                auto height = com["height"].asInt();

                auto atlas = GTUIManager::FindAtlas(sprite_atlas);
                if(atlas)
                {
                    int pivot_x = pivot % 3;
                    int pivot_y = pivot / 3;

                    auto sprite = atlas->CreateSprite(
                        sprite_name,
                        Vector2(0.5f * pivot_x, 0.5f * pivot_y),
                        pixel_per_unit,
                        (Sprite::Type::Enum) sprite_type,
                        Vector2((float) width, (float) height));
                    sprite->SetFlip((Sprite::Flip::Enum) sprite_flip);
                    sprite->SetFillDirection((Sprite::FillDirection::Enum) sprite_fill_dir);
                    sprite->SetFillAmount(sprite_fill_amount);
                    sprite->SetFillInvert(sprite_fill_invert);

                    auto sprite_node = obj->AddComponent<SpriteNode>();
                    sprite_node->SetSprite(sprite);
                    sprite_node->SetSortingOrder(depth);
                    sprite_node->SetColor(color);

                    batch->AddSprite(sprite_node);
                }
            }
            else if(type == "Label")
            {
                auto font = com["font"].asString();
                auto font_size = com["font_size"].asInt();
                //auto font_style = com["font_style"].asString();
                auto text = com["text"].asString();
                auto overflow = com["overflow"].asString();
                auto alignment = com["alignment"].asInt();
                //auto effect_style = com["effect_style"].asString();
                //auto effect_color = read_color(com, "effect_color");
                auto space_x = com["space_x"].asInt();
                auto space_y = com["space_y"].asInt();
                //auto max_line = com["max_line"].asInt();
                auto rich = com["rich"].asBool();
                auto color = read_color(com, "color");
                auto pivot = com["pivot"].asInt();
                auto depth = com["depth"].asInt();
                auto width = com["width"].asInt();
                auto height = com["height"].asInt();

                auto label = Label::Create(text, font, font_size, (LabelPivot::Enum) pivot, (LabelAlign::Enum) alignment, rich);
                if(label)
                {
                    label->SetCharSpace(space_x);
                    label->SetLineSpace(space_y);
                    if(overflow == "ClampContent")
                    {
                        label->SetWidth(width);
                        label->SetHeight(height);
                    }
                    auto tr = obj->AddComponent<TextRenderer>();
                    tr->SetLabel(label);
                    tr->SetColor(color);
                    tr->SetSortingOrder(sorting_layer, depth);
                }
            }
            else if(type == "BoxCollider")
            {
                auto center = read_vector3(com, "center");
                auto size = read_vector3(com, "size");

                auto collider = obj->AddComponent<BoxCollider>();
                collider->SetCenter(center);
                collider->SetSize(size);
            }
        }

        auto children = node["children"];
        for(size_t i=0; i<children.size(); i++)
        {
            auto child = read_window_json_node(children[i], transform, sorting_layer, batch, pixel_per_unit);
        }

        return obj;
    }

    std::shared_ptr<GameObject> GTUIManager::LoadWindowFromJsonFile(const std::string &file, int sorting_layer, float pixel_per_unit)
    {
        std::shared_ptr<GameObject> obj;

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
                    obj = read_window_json_node(root, std::shared_ptr<Transform>(), sorting_layer, std::shared_ptr<SpriteBatchRenderer>(), pixel_per_unit);
                }
            }
        }

        return obj;
    }
}
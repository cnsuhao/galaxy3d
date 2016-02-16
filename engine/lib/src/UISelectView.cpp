#include "UISelectView.h"
#include "GameObject.h"
#include "UIAtlas.h"

namespace Galaxy3D
{
    struct UISelectItemEventListener : public UIEventListener
    {
        int index;
        std::weak_ptr<SpriteNode> high_light;
        std::weak_ptr<UISelectView> select_view;

        virtual void OnHover(bool hover)
        {
            auto sprite = high_light.lock();

            if(sprite)
            {
                Color c = sprite->GetColor();
                if(hover)
                {
                    c.a = 1.0f;
                }
                else
                {
                    c.a = 0.0f;
                }

                sprite->SetColor(c);
            }
        }

        virtual void OnClick()
        {
            if(!select_view.expired())
            {
                select_view.lock()->Select(index);
            }
        }
    };

    void UISelectView::Start()
    {
        for(size_t i=0; i<items.size(); i++)
        {
            auto &item = items[i];

            auto handler = item.high_light->GetGameObject()->AddComponent<UISelectItemEventListener>();
            handler->index = i;
            handler->high_light = item.high_light;
            handler->select_view = std::dynamic_pointer_cast<UISelectView>(GetComponentPtr());
        }
    }

    void UISelectView::Select(int index)
    {
        if(index >= 0 && index < (int) items.size())
        {
            selected_index = index;
            if(selected_item)
            {
                selected_item->SetText(items[index].label->GetText());
            }

            if(list->IsActiveSelf())
            {
                OnClick();
            }
            
            OnSelected(selected_index);
        }
    }

    void UISelectView::OnClick()
    {
        auto sprite = arrow->GetSprite();
        if(sprite)
        {
            auto atlas = sprite->GetAtlas().lock();
            if(atlas)
            {
                if(!arrow_up.empty() && !arrow_down.empty())
                {
                    atlas->SetSpriteData(sprite, list->IsActiveSelf() ? arrow_down : arrow_up);
                }
            }
        }

        list->SetActive(!list->IsActiveSelf());
    }
}
#ifndef __UIButton_h__
#define __UIButton_h__

#include "UIEventListener.h"
#include "Sprite.h"
#include "UIAtlas.h"

namespace Galaxy3D
{
    struct UIButton : public UIEventListener
    {
        std::shared_ptr<Sprite> sprite;
        std::string sprite_name_normal;
        std::string sprite_name_pressed;

        virtual void OnPress(bool press)
        {
            if(!sprite)
            {
                return;
            }

            auto atlas = sprite->GetAtlas().lock();
            if(!atlas)
            {
                return;
            }

            if(press)
            {
                if(!sprite_name_pressed.empty())
                {
                    atlas->SetSpriteData(sprite, sprite_name_pressed);
                }
            }
            else
            {
                if(!sprite_name_normal.empty())
                {
                    atlas->SetSpriteData(sprite, sprite_name_normal);
                }
            }
        }
    };
}

#endif
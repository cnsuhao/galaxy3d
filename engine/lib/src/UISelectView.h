#ifndef __UISelectView_h__
#define __UISelectView_h__

#include "UIEventListener.h"
#include "Label.h"
#include "SpriteNode.h"

namespace Galaxy3D
{
    struct UISelectItem
    {
        std::shared_ptr<Label> label;
        std::shared_ptr<SpriteNode> high_light;
    };

    class UISelectView : public UIEventListener
    {
    public:
        std::shared_ptr<Label> selected_item;
        int selected_index;
        std::shared_ptr<SpriteNode> arrow;
        std::string arrow_down;
        std::string arrow_up;
        std::shared_ptr<GameObject> list;
        std::vector<UISelectItem> items;

        virtual void OnClick();
        virtual void OnSelected(int index) {}
        void Select(int index);

    protected:
        virtual void Start();
    };
}

#endif
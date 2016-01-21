#ifndef __UIEventListener_h__
#define __UIEventListener_h__

#include "Vector3.h"
#include "Component.h"

namespace Galaxy3D
{
    class UIEventListener : public Component
    {
    public:
        virtual void OnHover(bool hover) {}
        virtual void OnPress(bool press) {}
        virtual void OnLongPress() {}
        virtual void OnSelect(bool select) {}
        virtual void OnClick() {}
        virtual void OnDoubleClick() {}

        virtual void OnDragStart() {}
        virtual void OnDrag(const Vector3 &delta) {}
        virtual void OnDragOver(std::weak_ptr<GameObject> &dragged) {}
        virtual void OnDragOut(std::weak_ptr<GameObject> &dragged) {}
        virtual void OnDragEnd() {}
        virtual void OnDrop(std::weak_ptr<GameObject> &dragged) {}
    };
}

#endif
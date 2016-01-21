#ifndef __UIWidget_h__
#define __UIWidget_h__

#include "Vector3.h"

namespace Galaxy3D
{
    class GameObject;

    class UIWidget
    {
    public:
        virtual void OnHover(bool hover) {}
        virtual void OnPress(bool press) {}
        virtual void OnSelect(bool select) {}
        virtual void OnClick() {}
        virtual void OnDoubleClick() {}

        virtual void OnDragStart() {}
        virtual void OnDrag(const Vector3 &delta) {}
        virtual void OnDragOver(std::weak_ptr<GameObject> &dragged) {}
        virtual void OnDragOut(std::weak_ptr<GameObject> &dragged) {}
        virtual void OnDragEnd() {}
    };
}

#endif
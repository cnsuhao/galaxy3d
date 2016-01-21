#ifndef __ButtonEventListener_h__
#define __ButtonEventListener_h__

#include "UIEventListener.h"
#include "Debug.h"

namespace Galaxy3D
{
    class ButtonEventListener : public UIEventListener
    {
    public:
        virtual void OnHover(bool hover) {Debug::Log("OnHover");}
        virtual void OnPress(bool press) {Debug::Log("OnPress");}
        virtual void OnLongPress() {Debug::Log("OnLongPress");}
        virtual void OnSelect(bool select) {Debug::Log("OnSelect");}
        virtual void OnClick() {Debug::Log("OnClick");}
        virtual void OnDoubleClick() {Debug::Log("OnDoubleClick");}

        virtual void OnDragStart() {Debug::Log("OnDragStart");}
        virtual void OnDrag(const Vector3 &delta) {Debug::Log("OnDrag");}
        virtual void OnDragOver(std::weak_ptr<GameObject> &dragged) {Debug::Log("OnDragOver");}
        virtual void OnDragOut(std::weak_ptr<GameObject> &dragged) {Debug::Log("OnDragOut");}
        virtual void OnDragEnd() {Debug::Log("OnDragEnd");}
        virtual void OnDrop(std::weak_ptr<GameObject> &dragged) {Debug::Log("OnDrop");}
    };
}

#endif
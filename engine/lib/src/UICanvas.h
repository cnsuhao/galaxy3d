#ifndef __UICanvas_h__
#define __UICanvas_h__

#include "Component.h"
#include "Vector4.h"
#include "Camera.h"

namespace Galaxy3D
{
    class UICanvas : public Component
    {
    public:
        UICanvas():
            m_width(0),
            m_height(0)
        {}
        void SetSize(int w, int h);
        int GetWidth();
        int GetHeight();
        void AnchorTransform(std::shared_ptr<Transform> &t, const Vector4 &anchor);

    protected:
        virtual void Start();
        virtual void Update();

    private:
        int m_width;
        int m_height;
        std::shared_ptr<Camera> m_camera;

        void ProcessMouse();
        void ProcessTouch(bool pressed, bool released);
        void SetHoveredObject(std::weak_ptr<GameObject> &obj);
    };
}

#endif
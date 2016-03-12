#ifndef __UICanvas_h__
#define __UICanvas_h__

#include "Component.h"
#include "Vector4.h"
#include "Camera.h"
#include "Physics.h"

namespace Galaxy3D
{
    class UICanvas : public Component
    {
    public:
        static std::weak_ptr<GameObject> GetRayHitObject();
        static Vector2 GetLastPosition();
        static RaycastHit GetLastRayHit();
		static bool IsPressStarted();
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
        void ProcessRelease(float drag);
        void ProcessPress(bool pressed, float click, float drag);
        void SetHoveredObject(const std::weak_ptr<GameObject> &obj);
    };
}

#endif
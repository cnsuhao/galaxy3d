#ifndef __UICanvas_h__
#define __UICanvas_h__

#include "Component.h"
#include "Vector4.h"

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
        virtual void Update();

    private:
        int m_width;
        int m_height;

        void ProcessMouse();
    };
}

#endif
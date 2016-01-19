#ifndef __UICanvas_h__
#define __UICanvas_h__

#include "Component.h"

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

    protected:
        virtual void Start();

    private:
        int m_width;
        int m_height;
    };
}

#endif
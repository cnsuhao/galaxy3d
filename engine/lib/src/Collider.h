#ifndef __Collider_h__
#define __Collider_h__

#include "Component.h"

namespace Galaxy3D
{
    class Collider : public Component
    {
    protected:
        void *m_rigidbody;

        Collider():
            m_rigidbody(NULL)
        {}
        virtual void OnEnable();
        virtual void OnDisable();
    };
}

#endif
#ifndef __BoxCollider_h__
#define __BoxCollider_h__

#include "Collider.h"
#include "Vector3.h"

namespace Galaxy3D
{
    class BoxCollider : public Collider
    {
    public:
        BoxCollider():
            m_center(0, 0, 0),
            m_size(1, 1, 1)
        {}
        void SetCenter(const Vector3 &center) {m_center = center;}
        void SetSize(const Vector3 &size) {m_size = size;}

    protected:
        virtual void Start();
        virtual void OnTranformChanged();

    private:
        Vector3 m_center;
        Vector3 m_size;
    };
}

#endif
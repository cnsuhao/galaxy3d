#ifndef __TweenRotation_h__
#define __TweenRotation_h__

#include "Tweenner.h"
#include "Quaternion.h"

namespace Galaxy3D
{
    class TweenRotation : public Tweenner
    {
    public:
        TweenRotation():
            is_world(false),
            quaternion_slerp(false)
        {}

    protected:
        virtual void SetValue(float t);

    public:
        Vector3 from;
        Vector3 to;
        bool is_world;
        bool quaternion_slerp;
    };
}

#endif
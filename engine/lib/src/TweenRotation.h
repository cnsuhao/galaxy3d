#ifndef __TweenRotation_h__
#define __TweenRotation_h__

#include "Tweenner.h"
#include "Quaternion.h"

namespace Galaxy3D
{
    class TweenRotation : public Tweenner
    {
    public:
        TweenRotation() {}

    protected:
        virtual void SetValue(float t);

    public:
        Quaternion from;
        Quaternion to;
        bool is_world;
    };
}

#endif
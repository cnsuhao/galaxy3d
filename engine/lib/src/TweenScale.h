#ifndef __TweenScale_h__
#define __TweenScale_h__

#include "Tweenner.h"
#include "Vector3.h"

namespace Galaxy3D
{
    class TweenScale : public Tweenner
    {
    public:
        TweenScale():
            is_world(false)
        {}

    protected:
        virtual void SetValue(float t);

    public:
        Vector3 from;
        Vector3 to;
        bool is_world;
    };
}

#endif
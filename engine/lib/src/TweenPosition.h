#ifndef __TweenPosition_h__
#define __TweenPosition_h__

#include "Tweenner.h"
#include "Vector3.h"

namespace Galaxy3D
{
    class TweenPosition : public Tweenner
    {
    public:
        TweenPosition() {}

    protected:
        virtual void SetValue(float t);

    public:
        Vector3 from;
        Vector3 to;
        bool is_world;
    };
}

#endif
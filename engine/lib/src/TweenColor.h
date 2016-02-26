#ifndef __TweenColor_h__
#define __TweenColor_h__

#include "Tweenner.h"
#include "Color.h"

namespace Galaxy3D
{
    class TweenColor : public Tweenner
    {
    public:
        TweenColor()
        {}

    protected:
        virtual void SetValue(float t);

    public:
        Color from;
        Color to;
    };
}

#endif
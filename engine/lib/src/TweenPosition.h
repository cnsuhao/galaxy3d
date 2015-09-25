#ifndef __TweenPosition_h__
#define __TweenPosition_h__

#include "Component.h"
#include "Vector3.h"
#include "AnimationCurve.h"
#include "GTTime.h"

namespace Galaxy3D
{
    typedef void (*OnTweenFinished)(Component *com);

    class TweenPosition : public Component
    {
    public:
        TweenPosition():
            delay(0),
            duration(0),
            on_finished(NULL),
            is_world(false)
        {}

    protected:
        virtual void Start();
        virtual void Update();

    public:
        float delay;
        float duration;
        Vector3 from;
        Vector3 to;
        AnimationCurve curve;
        OnTweenFinished on_finished;
        bool is_world;

    protected:
        float m_time_start;
    };
}

#endif
#ifndef __TweenPosition_h__
#define __TweenPosition_h__

#include "Component.h"
#include "Vector3.h"
#include "AnimationCurve.h"
#include "GTTime.h"

namespace Galaxy3D
{
    typedef void (*OnTweenFinished)(Component *tween, std::weak_ptr<Component> &target);
    typedef void (*OnTweenSetValue)(Component *tween, std::weak_ptr<Component> &target, void *value);

    class TweenPosition : public Component
    {
    public:
        TweenPosition():
            delay(0),
            duration(0),
            on_set_value(NULL),
            on_finished(NULL),
            is_world(false)
        {}
        void Reset();

    protected:
        void SetPosition(float t);
        virtual void Start();
        virtual void Update();

    public:
        float delay;
        float duration;
        Vector3 from;
        Vector3 to;
        AnimationCurve curve;
        std::weak_ptr<Component> target;
        OnTweenSetValue on_set_value;
        OnTweenFinished on_finished;
        bool is_world;

    protected:
        float m_time_start;
    };
}

#endif
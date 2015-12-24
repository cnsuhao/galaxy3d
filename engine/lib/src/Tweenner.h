#ifndef __Tweenner_h__
#define __Tweenner_h__

#include "Component.h"
#include "AnimationCurve.h"

namespace Galaxy3D
{
    typedef void (*OnTweenFinished)(Component *tween, std::weak_ptr<Component> &target);
    typedef void (*OnTweenSetValue)(Component *tween, std::weak_ptr<Component> &target, void *value);

    class Tweenner : public Component
    {
    public:
        Tweenner():
            delay(0),
            duration(0),
            loop(false),
            on_set_value(NULL),
            on_finished(NULL)
        {}
        void Reset();

    protected:
        virtual void SetValue(float t) {}
        virtual void Start();
        virtual void Update();

    public:
        float delay;
        float duration;
        AnimationCurve curve;
        bool loop;
        std::weak_ptr<Component> target;
        OnTweenSetValue on_set_value;
        OnTweenFinished on_finished;

    protected:
        float m_time_start;
    };
}

#endif
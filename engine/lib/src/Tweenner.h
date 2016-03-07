#ifndef __Tweenner_h__
#define __Tweenner_h__

#include "Component.h"
#include "AnimationCurve.h"
#include <functional>

namespace Galaxy3D
{
    class Tweenner : public Component
    {
    public:
        Tweenner():
            delay(0),
            duration(0),
            loop(false)
        {}
        void Reset();
		void SetTime(float t);

    protected:
        virtual void SetValue(float t) {}
        virtual void Awake();
        virtual void Update();

    public:
        float delay;
        float duration;
        AnimationCurve curve;
        bool loop;
        std::weak_ptr<Component> target;
        std::function<void(Component *tween, std::weak_ptr<Component> &target, void *value)> on_set_value;
		std::function<void(Component *tween, std::weak_ptr<Component> &target)> on_finished;
    protected:
        float m_time_start;
    };
}

#endif
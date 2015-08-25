#ifndef __Animation_h__
#define __Animation_h__

#include "Component.h"
#include "AnimationState.h"
#include <unordered_map>

namespace Galaxy3D
{
    class Animation : public Component
    {
    public:
        Animation():
            m_wrap_mode(WrapMode::Default)
        {}
        virtual ~Animation() {}
        void SetAnimationStates(const std::unordered_map<std::string, AnimationState> &states) {m_states = states;}

    private:
        struct Blend
        {
            AnimationState *state;
            float weight;

            bool operator <(const Blend &b) const
            {
                return state->layer < b.state->layer;
            }

            Blend():
                state(0),
                weight(0)
            {}
        };

        std::unordered_map<std::string, AnimationState> m_states;
        WrapMode::Enum m_wrap_mode;
        std::list<Blend> m_blends;

        virtual void Start();
        virtual void Update();
        void UpdateBlend();
        void UpdateBones();
        void Stop(AnimationState &state);
    };
}

#endif
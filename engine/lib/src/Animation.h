#ifndef __Animation_h__
#define __Animation_h__

#include "Component.h"
#include "AnimationState.h"
#include <unordered_map>

namespace Galaxy3D
{
    struct PlayMode
    {
        enum Enum
        {
            StopSameLayer = 0,
            StopAll = 4,
        };
    };

    class Animation : public Component
    {
    public:
        Animation() {}
        virtual ~Animation() {}
        void SetAnimationStates(const std::unordered_map<std::string, AnimationState> &states) {m_states = states;}
        bool Play(const std::string &clip, PlayMode::Enum mode = PlayMode::StopSameLayer);
        AnimationState *GetAnimationState(const std::string &clip);

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
        std::list<Blend> m_blends;

        virtual void Start();
        virtual void Update();
        void UpdateBlend();
        void UpdateBones();
        void Play(AnimationState &state);
        void Stop(AnimationState &state);
    };
}

#endif
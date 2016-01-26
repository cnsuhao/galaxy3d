#ifndef __AnimationState_h__
#define __AnimationState_h__

#include "Component.h"
#include "AnimationClip.h"

namespace Galaxy3D
{
    struct AnimationBlendMode
    {
        enum Enum
        {
            Blend = 0,
            Additive = 1,
        };
    };

    struct FadeMode
    {
        enum Enum
        {
            None,
            In,
            Out,
        };
    };

    struct AnimationFade
    {
        FadeMode::Enum mode;
        float length;
        float weight;

        AnimationFade():
            mode(FadeMode::None),
            length(0),
            weight(0)
        {}

        void Clear()
        {
            mode = FadeMode::None;
            length = 0;
            weight = 0;
        }
    };

    struct AnimationState
    {
        AnimationState(){}
        AnimationState(std::shared_ptr<AnimationClip> &clip):
            clip(clip),
            name(clip->GetName()),
            blend_mode(AnimationBlendMode::Blend),
            enabled(false),
            layer(0),
            length(clip->length),
            normalized_speed(1 / clip->length),
            normalized_time(0),
            speed(1),
            time(0),
            weight(1),
            wrap_mode(WrapMode::Default),
            time_last(0),
            play_dir(1)
        {
        }

        std::string name;
        std::shared_ptr<AnimationClip> clip;
        AnimationBlendMode::Enum blend_mode;
        bool enabled;
        int layer;
        float length;
        float normalized_speed;
        float normalized_time;
        float speed;
        float time;
        float weight;
        WrapMode::Enum wrap_mode;

        float time_last;
        int play_dir;
        AnimationFade fade;
    };
}

#endif
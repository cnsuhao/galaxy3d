#ifndef __AnimationClip_h__
#define __AnimationClip_h__

#include "Component.h"
#include "AnimationCurve.h"
#include "WrapMode.h"
#include "Transform.h"
#include <unordered_map>

namespace Galaxy3D
{
    struct CurveProperty
    {
        enum Enum
        {
            LocalPosX,
            LocalPosY,
            LocalPosZ,
            LocalRotX,
            LocalRotY,
            LocalRotZ,
            LocalRotW,
            LocalScaX,
            LocalScaY,
            LocalScaZ,

            Count
        };
    };

    struct CurveBinding
    {
        std::string path;
        std::shared_ptr<Transform> transform;

        std::vector<AnimationCurve> curves;
    };

    class AnimationClip : public Object
    {
    public:
        float frame_rate;
        float length;
        WrapMode::Enum wrap_mode;

        std::unordered_map<std::string, CurveBinding> curves;
    };
}

#endif
#ifndef __AnimationCurve_h__
#define __AnimationCurve_h__

#include "WrapMode.h"
#include <vector>

namespace Galaxy3D
{
    struct Keyframe
    {
        Keyframe(){}
        Keyframe(float time, float value):in_tangent(0),out_tangent(0),tangent_mode(0),time(time),value(value){}
        Keyframe(float time, float value, float in_tangent, float out_tangent):in_tangent(in_tangent),out_tangent(out_tangent),tangent_mode(0),time(time),value(value){}

        float in_tangent;
        float out_tangent;
        int tangent_mode;
        float time;
        float value;
    };

    struct AnimationCurve
    {
        std::vector<Keyframe> keys;

        float Evaluate(float time);
    };
}

#endif
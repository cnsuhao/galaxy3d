#include "AnimationCurve.h"
#include <math.h>
#include "Mathf.h"

namespace Galaxy3D
{
    static float evaluate(float time, const Keyframe &k0, const Keyframe &k1)
    {
        float dt = k1.time - k0.time;
        if(fabs(dt) < Mathf::Epsilon)
        {
            return k0.value;
        }

        float t = (time - k0.time) / dt;
        float t2 = t * t;
        float t3 = t2 * t;
        float _t = 1 - t;
        float _t2 = _t * _t;
        float _t3 = _t2 * _t;

        float c = 1 / 3.0f;
        float c0 = dt * c * k0.out_tangent + k0.value;
        float c1 = - dt * c * k1.in_tangent + k1.value;
        float value = k0.value * _t3 + 3 * c0 * t * _t2 + 3 * c1 * t2 * _t + k1.value * t3;

        return value;
    }

    float AnimationCurve::Evaluate(float time)
    {
        if(keys.empty())
        {
            return 0;
        }

        const Keyframe &back = keys.back();
        if(time >= back.time)
        {
            return back.value;
        }

        for(size_t i=0; i<keys.size(); i++)
        {
            const Keyframe &frame = keys[i];

            if(time < frame.time)
            {
                if(i == 0)
                {
                    return frame.value;
                }
                else
                {
                    return evaluate(time, keys[i - 1], frame);
                }
            }
        }

        return 0;
    }
}
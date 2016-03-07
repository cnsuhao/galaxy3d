#include "TweenScale.h"
#include "Transform.h"

namespace Galaxy3D
{
    void TweenScale::SetValue(float t)
    {
        float value = curve.Evaluate(t);

        Vector3 scale = Vector3::Lerp(from, to, value, false);
        if(on_set_value)
        {
            on_set_value(this, target, &scale);
        }
        else
        {
            if(is_world)
            {
                GetTransform()->SetScale(scale);
            }
            else
            {
                GetTransform()->SetLocalScale(scale);
            }
        }
    }
}
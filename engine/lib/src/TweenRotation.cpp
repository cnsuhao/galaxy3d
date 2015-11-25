#include "TweenRotation.h"
#include "Transform.h"

namespace Galaxy3D
{
    void TweenRotation::SetValue(float t)
    {
        float value = curve.Evaluate(t);

        Quaternion rot = Quaternion::Lerp(from, to, value);
        if(on_set_value != NULL)
        {
            on_set_value(this, target, &rot);
        }
        else
        {
            if(is_world)
            {
                GetTransform()->SetRotation(rot);
            }
            else
            {
                GetTransform()->SetLocalRotation(rot);
            }
        }
    }
}
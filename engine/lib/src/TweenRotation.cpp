#include "TweenRotation.h"
#include "Transform.h"

namespace Galaxy3D
{
    void TweenRotation::SetValue(float t)
    {
        float value = curve.Evaluate(t);

        Quaternion rot;
        Vector3 euler;
        if(quaternion_slerp)
        {
            rot = Quaternion::SLerp(Quaternion::Euler(from), Quaternion::Euler(to), value);
        }
        else
        {
            euler = Vector3::Lerp(from, to, value);
            rot = Quaternion::Euler(euler);
        }

        if(on_set_value != NULL)
        {
            if(quaternion_slerp)
            {
                on_set_value(this, target, &rot);
            }
            else
            {
                on_set_value(this, target, &euler);
            }
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
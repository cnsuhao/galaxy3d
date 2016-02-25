#include "TweenScale.h"
#include "Transform.h"

namespace Galaxy3D
{
    void TweenScale::SetValue(float t)
    {
        float value = curve.Evaluate(t);

        Vector3 pos = Vector3::Lerp(from, to, value, false);
        if(on_set_value != NULL)
        {
            on_set_value(this, target, &pos);
        }
        else
        {
            if(is_world)
            {
                GetTransform()->SetScale(pos);
            }
            else
            {
                GetTransform()->SetLocalScale(pos);
            }
        }
    }
}
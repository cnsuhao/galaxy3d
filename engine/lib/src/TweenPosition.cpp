#include "TweenPosition.h"
#include "Transform.h"

namespace Galaxy3D
{
    void TweenPosition::SetValue(float t)
    {
        float value = curve.Evaluate(t);

        Vector3 pos = Vector3::Lerp(from, to, value, false);
        if(on_set_value)
        {
            on_set_value(this, target, &pos);
        }
        else
        {
            if(is_world)
            {
                GetTransform()->SetPosition(pos);
            }
            else
            {
                GetTransform()->SetLocalPosition(pos);
            }
        }
    }
}
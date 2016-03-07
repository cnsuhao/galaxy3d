#include "TweenColor.h"

namespace Galaxy3D
{
    void TweenColor::SetValue(float t)
    {
        float value = curve.Evaluate(t);

        Color color = Color::Lerp(from, to, value, false);
        if(on_set_value)
        {
            on_set_value(this, target, &color);
        }
    }
}
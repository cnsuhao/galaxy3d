#include "TweenPosition.h"
#include "GameObject.h"

namespace Galaxy3D
{
    void TweenPosition::Start()
    {
        Reset();
    }

    void TweenPosition::Reset()
    {
        m_time_start = GTTime::GetRealTimeSinceStartup();
    }

    void TweenPosition::SetPosition(float t)
    {
        float value = curve.Evaluate(t);

        Vector3 pos = Vector3::Lerp(from, to, value);
        if(is_world)
        {
            GetTransform()->SetPosition(pos);
        }
        else
        {
            GetTransform()->SetLocalPosition(pos);
        }
    }

    void TweenPosition::Update()
    {
        float now = GTTime::GetRealTimeSinceStartup();
        if(now - m_time_start - delay >= duration)
        {
            SetPosition(1);

            if(on_finished != NULL)
            {
                on_finished(this, target);
            }

            Component::Destroy(GetComponentPtr());
        }
        else if(now - m_time_start >= delay)
        {
            float time = now - m_time_start - delay;

            SetPosition(time / duration);
        }
    }
}
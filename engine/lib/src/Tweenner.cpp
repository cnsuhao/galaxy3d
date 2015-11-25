#include "Tweenner.h"
#include "GTTime.h"

namespace Galaxy3D
{
    void Tweenner::Start()
    {
        Reset();
    }

    void Tweenner::Reset()
    {
        m_time_start = GTTime::GetRealTimeSinceStartup();
    }

    void Tweenner::Update()
    {
        float now = GTTime::GetRealTimeSinceStartup();
        if(now - m_time_start - delay >= duration)
        {
            SetValue(1);

            if(on_finished != NULL)
            {
                on_finished(this, target);
            }

            Component::Destroy(GetComponentPtr());
        }
        else if(now - m_time_start >= delay)
        {
            float time = now - m_time_start - delay;

            SetValue(time / duration);
        }
    }
}
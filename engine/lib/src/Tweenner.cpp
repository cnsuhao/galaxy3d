#include "Tweenner.h"
#include "GTTime.h"

namespace Galaxy3D
{
    void Tweenner::Awake()
    {
		Reset();
	}

	void Tweenner::SetTime(float t)
	{
		m_time_start = GTTime::GetRealTimeSinceStartup() - t;
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

            if(loop)
            {
                Reset();
            }
            else
            {
                if(on_finished)
                {
                    on_finished(this, target);
                }

				auto c = GetComponentPtr();
                Component::Destroy(c);
            }
        }
        else if(now - m_time_start >= delay)
        {
            float time = now - m_time_start - delay;

            SetValue(time / duration);
        }
    }
}
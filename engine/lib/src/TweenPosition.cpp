#include "TweenPosition.h"
#include "GameObject.h"

namespace Galaxy3D
{
    void TweenPosition::Start()
    {
        m_time_start = GTTime::GetRealTimeSinceStartup();
    }

    void TweenPosition::Update()
    {
        float now = GTTime::GetRealTimeSinceStartup();
        if(now - m_time_start - delay >= duration)
        {
            if(on_finished != NULL)
            {
                on_finished(this);
            }

            Component::Destroy(GetGameObject()->GetComponentPtr(this));
        }
        else if(now - m_time_start >= delay)
        {
            float time = now - m_time_start - delay;
            float value = curve.Evaluate(time / duration);

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
    }
}
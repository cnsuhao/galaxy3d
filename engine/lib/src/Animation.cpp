#include "Animation.h"
#include "GTTime.h"

namespace Galaxy3D
{
    void Animation::Start()
    {
    }

    void Animation::Update()
    {
        m_blends.clear();

        for(auto i = m_states.begin(); i != m_states.end(); i++)
        {
            AnimationState *state = &i->second;
            AnimationClip *c = &state->clip;

            if(!state->enabled)
            {
                continue;
            }

            float now = GTTime::GetRealTimeSinceStartup();
            float time = now - state->time_start;
            state->time += time * state->play_dir;
            state->time_start = now;

            if(state->fade.mode == FadeMode::In)
            {
                state->fade.weight += time * (state->weight - 0) / state->fade.length;
                if(state->fade.weight >= state->weight)
                {
                    state->fade.Clear();
                }
            }
            else if(state->fade.mode == FadeMode::Out)
            {
                state->fade.weight += time * (0 - state->weight) / state->fade.length;
                if(state->fade.weight <= 0)
                {
                    Stop(*state);
                }
            }

            if(	(state->play_dir == 1 && state->time > state->length) ||
                (state->play_dir == -1 && state->time < 0))
            {
                WrapMode::Enum wrap_mode = state->wrap_mode;
                if(wrap_mode == WrapMode::Default)
                {
                    wrap_mode = wrap_mode;
                }
                if(wrap_mode == WrapMode::Default)
                {
                    wrap_mode = c->wrap_mode;
                }

                switch(wrap_mode)
                {
                    case WrapMode::Default:
                    case WrapMode::Once:
                        Stop(*state);
                        continue;

                    case WrapMode::Loop:
                        state->time = 0;
                        break;

                    case WrapMode::PingPong:
                        if(state->play_dir == 1)
                        {
                            state->play_dir = -1;
                            state->time = state->length;
                        }
                        else
                        {
                            state->play_dir = 1;
                            state->time = 0;
                        }
                        break;

                    case WrapMode::ClampForever:
                        state->time = state->length;
                        break;
                }
            }

            if(state->enabled)
            {
                Blend blend;
                blend.state = state;
                m_blends.push_back(blend);
            }
        }

        UpdateBlend();
        UpdateBones();
    }

    void Animation::UpdateBlend()
    {
        float full_weight = 1.0f;
        float remain_weight = 1.0f;
        int layer = 0x7fffffff;

        //compute weights
        m_blends.sort();
        for(auto i = m_blends.begin(); i != m_blends.end(); i++)
        {
            if(remain_weight <= 0)
            {
                i->weight = 0;
                continue;	
            }

            if(i->state->layer < layer)
            {
                full_weight = remain_weight;
            }
            layer = i->state->layer;

            float weight;
            if(i->state->fade.mode != FadeMode::None)
            {
                weight = full_weight * i->state->fade.weight;
            }
            else
            {
                weight = full_weight * i->state->weight;
            }

            {
                i++;
                if(i == m_blends.end())
                {
                    weight = remain_weight;
                }
                i--;
            }

            if(remain_weight - weight < 0)
            {
                weight = remain_weight;
            }
            remain_weight -= weight;

            i->weight = weight;
        }
    }

    void Animation::UpdateBones()
    {
        Vector3 pos(0, 0, 0);
        Quaternion rot(0, 0, 0, 0);
        Vector3 sca(0, 0, 0);

        for(auto i = m_blends.begin(); i != m_blends.end(); i++)
        {
            auto state = i->state;
            float weight = i->weight;

            for(auto j = state->clip.curves.begin(); j != state->clip.curves.end(); j++)
            {

                //j->second.transform
            }
        }
    }

    void Animation::Stop(AnimationState &state)
    {
        state.enabled = false;
        state.time = 0;
        state.fade.Clear();
    }
}
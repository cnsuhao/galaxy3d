#include "ParticleSystem.h"
#include "GTTime.h"
#include "Debug.h"

namespace Galaxy3D
{
    ParticleSystem::~ParticleSystem()
    {
    }

    void ParticleSystem::Start()
    {
    }

    void ParticleSystem::Update()
    {
        UpdateEmitter();
        UpdateParticles();
    }

    void ParticleSystem::UpdateEmitter()
    {
        float t = GTTime::GetTime();

        if((t > start_delay) && ((t - start_delay) * playback_speed < duration || loop))//system ok
        {
            if((t - m_time_emit) * playback_speed > 1 / emission_rate || m_time_emit < 0)//time ok
            {
                if(enable_emission)//emission ok
                {
                    if(m_particles.size() < (size_t) max_particles)//count ok
                    {
                        if(emitter_shape == ParticleEmitterShape::Box)
                        {
                            float x = (rand() / (float) RAND_MAX - 0.5f) * emitter_shape_box_size.x;
                            float y = (rand() / (float) RAND_MAX - 0.5f) * emitter_shape_box_size.y;
                            float z = (rand() / (float) RAND_MAX - 0.5f) * emitter_shape_box_size.z;

                            Emit(Vector3(x, y, z), Vector3(0, 0, 1) * start_speed, start_size, start_lifetime, start_color);
                        }

                        m_time_emit = t;
                    }
                }
            }

            time = t - start_delay;
        }
    }

    void ParticleSystem::Emit(const Vector3 &position, const Vector3 &velocity, float size, float lifetime, const Color &color)
    {
        Particle p;
        p.angular_velocity = 0;
        p.axis_of_rotation = Vector3(0, 0, 1);
        p.color = color;
        p.start_lifetime = lifetime;
        p.lifetime = lifetime;
        p.position = position;
        p.rotation = start_rotation;
        p.size = size;
        p.velocity = velocity;

        m_particles.push_back(p);
    }

    void ParticleSystem::UpdateParticles()
    {
        float delta_time = GTTime::GetDeltaTime();

        for(auto i=m_particles.begin(); i!=m_particles.end(); )
        {
            i->position += i->velocity * delta_time;
            i->lifetime -= delta_time;

            if(i->lifetime < 0)
            {
                i = m_particles.erase(i);
                continue;
            }
            else
            {
                i++;
            }
        }
    }
}
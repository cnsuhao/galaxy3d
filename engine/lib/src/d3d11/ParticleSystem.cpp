#include "ParticleSystem.h"
#include "GTTime.h"
#include "VertexType.h"
#include "GameObject.h"
#include "ParticleSystemRenderer.h"
#include "Debug.h"

namespace Galaxy3D
{
    ParticleSystem::~ParticleSystem()
    {
        SAFE_RELEASE(m_vertex_buffer);
        SAFE_RELEASE(m_index_buffer);
    }

    void ParticleSystem::Awake()
    {
        auto psr = GetGameObject()->AddComponent<ParticleSystemRenderer>();
        psr->particle_system = GetGameObject()->GetComponent<ParticleSystem>();
    }

    ID3D11Buffer *ParticleSystem::GetVertexBuffer()
    {
        if(m_vertex_buffer == NULL)
        {
            int particle_max = (int) (emission_rate * start_lifetime);

            int buffer_size = sizeof(VertexUI) * 4 * particle_max;
            char *buffer = (char *) malloc(buffer_size);

            bool dynamic = true;

            auto device = GraphicsDevice::GetInstance()->GetDevice();

            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
            bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.ByteWidth = buffer_size;

            D3D11_SUBRESOURCE_DATA data;
            ZeroMemory(&data, sizeof(data));
            data.pSysMem = buffer;
            HRESULT hr = device->CreateBuffer(&bd, &data, &m_vertex_buffer);

            free(buffer);
        }

        return m_vertex_buffer;
    }

    ID3D11Buffer *ParticleSystem::GetIndexBuffer()
    {
        if(m_index_buffer == NULL)
        {
            int particle_max = (int) (emission_rate * start_lifetime);

            int buffer_size = sizeof(unsigned short) * 6 * particle_max;
            char *buffer = (char *) malloc(buffer_size);
            unsigned short *p = (unsigned short *) buffer;

            for(int i=0; i<particle_max; i+=6)
            {
                p[i] = i;
                p[i+1] = i+1;
                p[i+2] = i+2;
                p[i+3] = i;
                p[i+4] = i+2;
                p[i+5] = i+3;
            }

            auto device = GraphicsDevice::GetInstance()->GetDevice();

            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.CPUAccessFlags = 0;
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.ByteWidth = buffer_size;

            D3D11_SUBRESOURCE_DATA data;
            ZeroMemory(&data, sizeof(data));
            data.pSysMem = buffer;
            HRESULT hr = device->CreateBuffer(&bd, &data, &m_index_buffer);

            free(buffer);
        }

        return m_index_buffer;
    }

    void ParticleSystem::Update()
    {
        UpdateEmitter();
        UpdateParticles();
        UpdateBuffer();
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

            time = (t - start_delay) * playback_speed;
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
        float delta_time = GTTime::GetDeltaTime() * playback_speed;

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

    void ParticleSystem::UpdateBuffer()
    {

    }
}
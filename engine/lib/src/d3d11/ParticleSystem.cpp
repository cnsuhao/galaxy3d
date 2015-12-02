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

    int ParticleSystem::GetParticleCountMax() const
    {
        return Mathf::Min(Mathf::RoundToInt(emission_rate * start_lifetime * 1.1f), max_particles);
    }

    void ParticleSystem::UpdateBuffer()
    {
        GetVertexBuffer();

        int particle_count = GetParticleCount();

        if(m_vertex_buffer != NULL && particle_count > 0)
        {
            int buffer_size = sizeof(VertexUI) * 4 * particle_count;
            char *buffer = (char *) malloc(buffer_size);
            VertexUI *p = (VertexUI *) buffer;

            int j=0;
            for(auto i=m_particles.begin(); i!=m_particles.end(); i++)
            {
                VertexUI &v = p[j*4];
                VertexUI &v1 = p[j*4+1];
                VertexUI &v2 = p[j*4+2];
                VertexUI &v3 = p[j*4+3];

                v.COLOR = i->color;
                v1.COLOR = i->color;
                v2.COLOR = i->color;
                v3.COLOR = i->color;

                if(enable_texture_sheet_animation)
                {
                    float cycle_len = i->start_lifetime / texture_sheet_animation_cycles;
                    float t = fmodf(i->start_lifetime - i->lifetime, cycle_len) / cycle_len;
                    int frame = (int) texture_sheet_animation_frame_curve.Evaluate(t);
                    int x, y;

                    if(texture_sheet_animation_single_row)
                    {
                        x = frame;

                        if(texture_sheet_animation_random_row)
                        {
                            y = (int) ((rand() - 1) / (float) RAND_MAX * texture_sheet_animation_tile_y);
                        }
                        else
                        {
                            y = Mathf::Min(texture_sheet_animation_row, texture_sheet_animation_tile_y - 1);
                        }
                    }
                    else
                    {
                        x = frame % texture_sheet_animation_tile_x;
                        y = frame / texture_sheet_animation_tile_x;
                    }

                    v.TEXCOORD0 = Vector2(x / (float) texture_sheet_animation_tile_x, y / (float) texture_sheet_animation_tile_y);
                    v1.TEXCOORD0 = Vector2(x / (float) texture_sheet_animation_tile_x, (y + 1) / (float) texture_sheet_animation_tile_y);
                    v2.TEXCOORD0 = Vector2((x + 1) / (float) texture_sheet_animation_tile_x, (y + 1) / (float) texture_sheet_animation_tile_y);
                    v3.TEXCOORD0 = Vector2((x + 1) / (float) texture_sheet_animation_tile_x, y / (float) texture_sheet_animation_tile_y);
                }
                else
                {
                    v.TEXCOORD0 = Vector2(0, 0);
                    v1.TEXCOORD0 = Vector2(0, 1);
                    v2.TEXCOORD0 = Vector2(1, 1);
                    v3.TEXCOORD0 = Vector2(1, 0);
                }

                v.POSITION = Vector3(-i->size * 0.5f, i->size * 0.5f, 0);
                v1.POSITION = Vector3(-i->size * 0.5f, -i->size * 0.5f, 0);
                v2.POSITION = Vector3(i->size * 0.5f, -i->size * 0.5f, 0);
                v3.POSITION = Vector3(i->size * 0.5f, i->size * 0.5f, 0);

                for(int k=0; k<4; k++)
                {
                    Vector3 pos = Quaternion::AngleAxis(-i->rotation, i->axis_of_rotation) * p[j*4+k].POSITION;
                    if(m_target_camera)
                    {
                        Quaternion rot = Quaternion::FromToRotation(GetTransform()->GetForward(), m_target_camera->GetTransform()->GetForward());
                        pos = rot * pos;
                    }
                    p[j*4+k].POSITION = pos + i->position;
                }

                j++;
            }

            auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

            D3D11_MAPPED_SUBRESOURCE dms;
            ZeroMemory(&dms, sizeof(dms));
            context->Map(m_vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
            memcpy(dms.pData, buffer, buffer_size);
            context->Unmap(m_vertex_buffer, 0);

            free(buffer);
        }
    }

    ID3D11Buffer *ParticleSystem::GetVertexBuffer()
    {
        if(m_vertex_buffer == NULL)
        {
            int particle_max = GetParticleCountMax();

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
            int particle_max = GetParticleCountMax();

            int buffer_size = sizeof(unsigned short) * 6 * particle_max;
            char *buffer = (char *) malloc(buffer_size);
            unsigned short *p = (unsigned short *) buffer;

            for(int i=0; i<particle_max; i++)
            {
                p[i*6] = i*4;
                p[i*6+1] = i*4+1;
                p[i*6+2] = i*4+2;
                p[i*6+3] = i*4;
                p[i*6+4] = i*4+2;
                p[i*6+5] = i*4+3;
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
                        Vector3 scale = GetTransform()->GetScale();
                        Vector3 position;
                        Vector3 velocity;
                        Color color;

                        if(emitter_shape == ParticleEmitterShape::Box)
                        {
                            float x = (rand() / (float) RAND_MAX - 0.5f) * emitter_shape_box_size.x;
                            float y = (rand() / (float) RAND_MAX - 0.5f) * emitter_shape_box_size.y;
                            float z = (rand() / (float) RAND_MAX - 0.5f) * emitter_shape_box_size.z;

                            position = Vector3(x * scale.x, y * scale.y, z * scale.z);

                            if(emitter_random_direction)
                            {
                                float dx, dy, dz;
                                do
                                {
                                    dx = rand() / (float) RAND_MAX - 0.5f;
                                    dy = rand() / (float) RAND_MAX - 0.5f;
                                    dz = rand() / (float) RAND_MAX - 0.5f;
                                }
                                while(Mathf::FloatEqual(dx, 0) && Mathf::FloatEqual(dy, 0) && Mathf::FloatEqual(dz, 0));

                                velocity = Vector3::Normalize(Vector3(dx, dy, dz)) * start_speed;
                            }
                            else
                            {
                                velocity = Vector3(0, 0, 1) * start_speed;
                            }
                        }
                        else if(emitter_shape == ParticleEmitterShape::Cone)
                        {
                            float y = (rand() / (float) RAND_MAX) * emitter_shape_cone_radius;
                            float theta = (rand() / (float) RAND_MAX) * 360;

                            Vector3 pos(0, y, 0);
                            Quaternion rot = Quaternion::Euler(0, 0, theta);
                            pos = rot * pos;

                            position = Vector3(pos.x * scale.x, pos.y * scale.y, pos.z * scale.z);

                            float angle = emitter_shape_cone_angle;
                            angle = Mathf::Max(angle, 1.0f);
                            angle = Mathf::Min(angle, 89.0f);

                            float h = emitter_shape_cone_radius / tanf(angle * Mathf::Deg2Rad);
                            Vector3 origin(0, 0, -h);

                            if(emitter_random_direction)
                            {
                                float y_dir = (rand() / (float) RAND_MAX) * emitter_shape_cone_radius;
                                float theta_dir = (rand() / (float) RAND_MAX) * 360;
                                Vector3 pos_dir = Vector3(0, y_dir, 0);
                                Quaternion rot_dir = Quaternion::Euler(0, 0, theta_dir);
                                pos_dir = rot_dir * pos_dir;

                                velocity = Vector3::Normalize(pos_dir - origin) * start_speed;
                            }
                            else
                            {
                                velocity = Vector3::Normalize(pos - origin) * start_speed;
                            }
                        }

                        if(start_color_gradient.HasKey())
                        {
                            color = start_color_gradient.GetColor(fmodf(time, duration) / duration);
                        }
                        else
                        {
                            color = start_color;
                        }

                        Emit(position, velocity, start_size, start_lifetime, color);

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
        p.axis_of_rotation = Vector3(0, 0, 1);
        p.start_color = color;
        p.color = color;
        p.start_lifetime = lifetime;
        p.lifetime = lifetime;
        p.position = position;
        p.rotation = start_rotation;
        p.start_size = size;
        p.size = size;
        p.velocity = velocity;

        m_particles.push_back(p);
    }

    void ParticleSystem::UpdateParticles()
    {
        float delta_time = GTTime::GetDeltaTime() * playback_speed;

        for(auto i=m_particles.begin(); i!=m_particles.end(); )
        {
            float t = (i->start_lifetime - i->lifetime) / i->start_lifetime;

            if(force_type == ForceType::Constant)
            {
                i->velocity += force * delta_time;
            }
            else if(force_type == ForceType::Curve)
            {
                float fx = force_curve_x.Evaluate(t);
                float fy = force_curve_y.Evaluate(t);
                float fz = force_curve_z.Evaluate(t);

                i->velocity += Vector3(fx, fy, fz) * delta_time;
            }

            i->position += i->velocity * delta_time;
            i->rotation += angular_velocity * delta_time;
            i->lifetime -= delta_time;

            if(color_gradient.HasKey())
            {
                i->color = i->start_color * color_gradient.GetColor(t);
            }

            if(!size_curve.keys.empty())
            {
                i->size = i->start_size * size_curve.Evaluate(t);
            }

            if(i->lifetime <= 0)
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
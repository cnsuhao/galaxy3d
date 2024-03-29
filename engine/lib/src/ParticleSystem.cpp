#include "ParticleSystem.h"
#include "GTTime.h"
#include "VertexType.h"
#include "GameObject.h"
#include "ParticleSystemRenderer.h"
#include "Debug.h"
#include "Mathf.h"

namespace Galaxy3D
{
    ParticleSystem::~ParticleSystem()
    {
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
    }

    void ParticleSystem::Awake()
    {
        auto psr = GetGameObject()->AddComponent<ParticleSystemRenderer>();
        psr->particle_system = GetGameObject()->GetComponent<ParticleSystem>();
        m_renderer = psr;
    }

    int ParticleSystem::GetParticleCountMax() const
    {
        float lifetime;

        if(start_lifetime_type == ValueType::RandomConstants)
        {
            lifetime = Mathf::Max(start_lifetime_random_contants.x, start_lifetime_random_contants.y);
        }
        else
        {
            lifetime = start_lifetime;
        }

        return Mathf::Min(Mathf::RoundToInt(emission_rate * lifetime * 1.1f), max_particles);
    }

    void ParticleSystem::UpdateBuffer()
    {
        GetVertexBuffer();

        int particle_count = GetParticleCount();

        if(m_vertex_buffer.buffer != NULL && particle_count > 0)
        {
            int buffer_size = sizeof(VertexUI) * 4 * particle_count;
            char *buffer = (char *) malloc(buffer_size);
            VertexUI *p = (VertexUI *) buffer;
            auto renderer = m_renderer.lock();
            Quaternion rot_to_cam;
            Quaternion rot_to_cam_stretch;
            Matrix4x4 local_to_world = Matrix4x4::TRS(GetTransform()->GetPosition(), GetTransform()->GetRotation(), Vector3(1, 1, 1));
            Matrix4x4 world_to_local = local_to_world.Inverse();
            Vector3 cam_foward_in_system = world_to_local.MultiplyDirection(m_target_camera->GetTransform()->GetForward());
            Vector3 cam_up_in_system = world_to_local.MultiplyDirection(m_target_camera->GetTransform()->GetUp());

            if(renderer->render_mode == ParticleSystemRenderMode::Stretch)
            {
                rot_to_cam_stretch = Quaternion::FromToRotation(Vector3(0, 1, 0) * -1.0f, cam_foward_in_system);
                rot_to_cam_stretch = Quaternion::FromToRotation(rot_to_cam_stretch * Vector3(0, 0, 1), cam_up_in_system) * rot_to_cam_stretch;
            }
            else
            {
                rot_to_cam = Quaternion::LookRotation(cam_foward_in_system, cam_up_in_system);
            }

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
                            y = Mathf::RandomRange(0, texture_sheet_animation_tile_y);
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

                Quaternion rot_velocity;
                float w, h;

                w = i->size;
                if(renderer->render_mode == ParticleSystemRenderMode::Stretch)
                {
                    float speed = i->velocity.Magnitude();
                    Vector3 velocity_wolrd = local_to_world.MultiplyDirection(i->velocity);
                    Vector3 velocity_view = m_target_camera->GetTransform()->InverseTransformDirection(velocity_wolrd);
                    float theta_velocity_y = atan2f(velocity_view.y, velocity_view.x) * Mathf::Rad2Deg - 90;
                    rot_velocity = Quaternion::Euler(0, -theta_velocity_y, 0);

                    h = renderer->stretch_speed_scale * speed + renderer->stretch_length_scale * w;
                    
                    v.POSITION = Vector3(-w * 0.5f, 0, h * 0.5f);
                    v1.POSITION = Vector3(-w * 0.5f, 0, -h * 0.5f);
                    v2.POSITION = Vector3(w * 0.5f, 0, -h * 0.5f);
                    v3.POSITION = Vector3(w * 0.5f, 0, h * 0.5f);
                }
                else
                {
                    h = w;

                    v.POSITION = Vector3(-w * 0.5f, h * 0.5f, 0);
                    v1.POSITION = Vector3(-w * 0.5f, -h * 0.5f, 0);
                    v2.POSITION = Vector3(w * 0.5f, -h * 0.5f, 0);
                    v3.POSITION = Vector3(w * 0.5f, h * 0.5f, 0);
                }

                auto rot_local = Quaternion::AngleAxis(-i->rotation, i->axis_of_rotation);
                for(int k=0; k<4; k++)
                {
                    Vector3 pos;
                    if(m_target_camera)
                    {
                        if(renderer->render_mode == ParticleSystemRenderMode::Stretch)
                        {
                            pos = rot_velocity * p[j*4+k].POSITION;
                            pos = rot_local * pos;
                            pos = rot_to_cam_stretch * pos;
                        }
                        else
                        {
                            pos = rot_local * p[j*4+k].POSITION;
                            pos = rot_to_cam * pos;
                        }
                    }

                    p[j*4+k].POSITION = pos + i->position;
                }

                j++;
            }

            GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);

            free(buffer);
        }
    }

    BufferObject ParticleSystem::GetVertexBuffer()
    {
        if(m_vertex_buffer.buffer == NULL)
        {
            int particle_max = GetParticleCountMax();

            int buffer_size = sizeof(VertexUI) * 4 * particle_max;
            char *buffer = (char *) malloc(buffer_size);

            m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);

            free(buffer);
        }

        return m_vertex_buffer;
    }

    BufferObject ParticleSystem::GetIndexBuffer()
    {
        if(m_index_buffer.buffer == NULL)
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

            m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);
           
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

    void ParticleSystem::EmitShapeNone(Vector3 &position, Vector3 &velocity, float speed)
    {
        position = Vector3(0, 0, 0);
        velocity = Vector3(0, 0, 1) * speed;
    }

    void ParticleSystem::EmitShapeSphere(Vector3 &position, Vector3 &velocity, float speed)
    {
        float dx, dy, dz;
        do
        {
            dx = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
            dy = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
            dz = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
        }
        while(Mathf::FloatEqual(dx, 0) && Mathf::FloatEqual(dy, 0) && Mathf::FloatEqual(dz, 0));

        float radius = Mathf::RandomRange(0.0f, emitter_shape_sphere_radius);
        Vector3 dir = Vector3::Normalize(Vector3(dx, dy, dz));
        position = dir * radius;

        if(emitter_random_direction)
        {
            do
            {
                dx = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
                dy = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
                dz = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
            }
            while(Mathf::FloatEqual(dx, 0) && Mathf::FloatEqual(dy, 0) && Mathf::FloatEqual(dz, 0));

            velocity = Vector3::Normalize(Vector3(dx, dy, dz)) * speed;
        }
        else
        {
            velocity = dir * speed;
        }
    }

    void ParticleSystem::EmitShapeCone(Vector3 &position, Vector3 &velocity, float speed)
    {
        float y = Mathf::RandomRange(0.0f, emitter_shape_cone_radius);
        float theta = Mathf::RandomRange(0.0f, 360.0f);

        Vector3 pos(0, y, 0);
        Quaternion rot = Quaternion::Euler(0, 0, theta);
        pos = rot * pos;

        position = pos;

        float angle = emitter_shape_cone_angle;
        angle = Mathf::Max(angle, 1.0f);
        angle = Mathf::Min(angle, 89.0f);

        float h = emitter_shape_cone_radius / tanf(angle * Mathf::Deg2Rad);
        Vector3 origin(0, 0, -h);

        if(emitter_random_direction)
        {
            float y_dir = Mathf::RandomRange(0.0f, emitter_shape_cone_radius);
            float theta_dir = Mathf::RandomRange(0.0f, 360.0f);
            Vector3 pos_dir = Vector3(0, y_dir, 0);
            Quaternion rot_dir = Quaternion::Euler(0, 0, theta_dir);
            pos_dir = rot_dir * pos_dir;

            velocity = Vector3::Normalize(pos_dir - origin) * speed;
        }
        else
        {
            velocity = Vector3::Normalize(pos - origin) * speed;
        }
    }

    void ParticleSystem::EmitShapeBox(Vector3 &position, Vector3 &velocity, float speed)
    {
        float x = (Mathf::RandomRange(0.0f, 1.0f) - 0.5f) * emitter_shape_box_size.x;
        float y = (Mathf::RandomRange(0.0f, 1.0f) - 0.5f) * emitter_shape_box_size.y;
        float z = (Mathf::RandomRange(0.0f, 1.0f) - 0.5f) * emitter_shape_box_size.z;

        position = Vector3(x, y, z);

        if(emitter_random_direction)
        {
            float dx, dy, dz;
            do
            {
                dx = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
                dy = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
                dz = Mathf::RandomRange(0.0f, 1.0f) - 0.5f;
            }
            while(Mathf::FloatEqual(dx, 0) && Mathf::FloatEqual(dy, 0) && Mathf::FloatEqual(dz, 0));

            velocity = Vector3::Normalize(Vector3(dx, dy, dz)) * speed;
        }
        else
        {
            velocity = Vector3(0, 0, 1) * speed;
        }
    }

    void ParticleSystem::UpdateEmitter()
    {
        float t = GTTime::GetTime();

        if((t > start_delay) && ((t - start_delay) * playback_speed < duration || loop))//system ok
        {
            if((((t - m_time_emit) * playback_speed) > (1 / emission_rate)) || (m_time_emit < 0))//time ok
            {
                if(enable_emission)//emission ok
                {
                    if(m_particles.size() < (size_t) max_particles)//count ok
                    {
                        Vector3 scale = GetTransform()->GetScale();
                        Vector3 position;
                        Vector3 velocity;
                        Color color;
                        float lifetime;
                        float speed;

                        if(start_speed_type == ValueType::RandomConstants)
                        {
                            speed = Mathf::RandomRange(0.0f, 1.0f) * (start_speed_random_contants.y - start_speed_random_contants.x) + start_speed_random_contants.x;
                        }
                        else
                        {
                            speed = start_speed;
                        }

                        switch(emitter_shape)
                        {
                            case ParticleEmitterShape::Sphere:
                                EmitShapeSphere(position, velocity, speed);
                                break;
                            case ParticleEmitterShape::Cone:
                                EmitShapeCone(position, velocity, speed);
                                break;
                            case ParticleEmitterShape::Box:
                                EmitShapeBox(position, velocity, speed);
                                break;
                            default:
                                EmitShapeNone(position, velocity, speed);
                                break;
                        }

                        position = Vector3(position.x * scale.x, position.y * scale.y, position.z * scale.z);

                        if(start_color_gradient.HasKey())
                        {
                            color = start_color_gradient.GetColor(fmodf(time, duration) / duration);
                        }
                        else
                        {
                            color = start_color;
                        }

                        if(start_lifetime_type == ValueType::RandomConstants)
                        {
                            lifetime = Mathf::RandomRange(0.0f, 1.0f) * (start_lifetime_random_contants.y - start_lifetime_random_contants.x) + start_lifetime_random_contants.x;
                        }
                        else
                        {
                            lifetime = start_lifetime;
                        }

                        Emit(position, velocity, start_size, lifetime, color);

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
			if(i->lifetime <= 0)
            {
                i = m_particles.erase(i);
                continue;
            }

            float t = (i->start_lifetime - i->lifetime) / i->start_lifetime;

            Vector3 v;

            if(force_type == ValueType::Constant)
            {
                i->velocity += force * delta_time;
            }
            else if(force_type == ValueType::Curve)
            {
                float fx = force_curve_x.Evaluate(t);
                float fy = force_curve_y.Evaluate(t);
                float fz = force_curve_z.Evaluate(t);

                i->velocity += Vector3(fx, fy, fz) * delta_time;
            }

            v = i->velocity;

            if(velocity_type == ValueType::Constant)
            {
                v += velocity;
            }
            else if(velocity_type == ValueType::Curve)
            {
                float vx = velocity_curve_x.Evaluate(t);
                float vy = velocity_curve_y.Evaluate(t);
                float vz = velocity_curve_z.Evaluate(t);

                v += Vector3(vx, vy, vz);
            }

            i->position += v * delta_time;
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

			i++;
        }
    }
}
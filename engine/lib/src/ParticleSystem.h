#ifndef __ParticleSystem_h__
#define __ParticleSystem_h__

#include "Component.h"
#include "Color.h"
#include "Vector3.h"
#include "Mathf.h"
#include <list>

namespace Galaxy3D
{
    struct Particle
    {
        Vector3 axis_of_rotation;
        float rotation;
        float angular_velocity;
        Color color;
        float start_lifetime;
        float lifetime;
        Vector3 position;
        Vector3 velocity;
        float size;
    };

    struct ParticleEmitterShape
    {
        enum Enum
        {
            Box,
        };
    };

    class ParticleSystem : public Component
    {
    public:
        float duration;
        bool loop;
        float emission_rate;
        bool enable_emission;
        float gravity_modifier;
        int max_particles;
        float playback_speed;
        bool local_space;
        Color start_color;
        float start_delay;
        float start_lifetime;
        float start_rotation;
        float start_size;
        float start_speed;
        float time;
        ParticleEmitterShape::Enum emitter_shape;
        Vector3 emitter_shape_box_size;

        ParticleSystem():
            start_delay(0),
            duration(5),
            loop(true),
            emission_rate(10),
            enable_emission(true),
            gravity_modifier(0),
            max_particles(1000),
            playback_speed(1),
            local_space(true),
            start_color(1, 1, 1 ,1),
            start_lifetime(5),
            start_rotation(0),
            start_size(1),
            start_speed(5),
            time(0),
            emitter_shape(ParticleEmitterShape::Box),
            emitter_shape_box_size(5, 5, 5),
            m_time_emit(-1)
        {}
        virtual ~ParticleSystem();

    protected:
        virtual void Start();
        virtual void Update();

    private:
        std::list<Particle> m_particles;
        float m_time_emit;

        void UpdateEmitter();
        void Emit(const Vector3 &position, const Vector3 &velocity, float size, float lifetime, const Color &color);
        void UpdateParticles();
    };
}

#endif
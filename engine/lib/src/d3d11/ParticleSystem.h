#ifndef __ParticleSystem_h__
#define __ParticleSystem_h__

#include "Component.h"
#include "Color.h"
#include "Vector3.h"
#include "Mathf.h"
#include "GraphicsDevice.h"
#include "Camera.h"
#include "AnimationCurve.h"
#include "ColorGradient.h"
#include <list>

namespace Galaxy3D
{
    struct Particle
    {
        Vector3 axis_of_rotation;
        float rotation;
        Color start_color;
        Color color;
        float start_lifetime;
        float lifetime;
        Vector3 position;
        Vector3 velocity;
        float start_size;
        float size;
    };

    struct ParticleEmitterShape
    {
        enum Enum
        {
            Disable,
            Sphere,
            Cone,
            Box,
        };
    };

    struct EmitterShapeConeFrom
    {
        enum Enum
        {
            Base,
        };
    };

    struct VelocityType
    {
        enum Enum
        {
            Disable,
            Constant,
            Curve,
        };
    };

    struct ForceType
    {
        enum Enum
        {
            Disable,
            Constant,
            Curve,
        };
    };

    class ParticleSystem : public Component
    {
    public:
        float duration;
        bool loop;
        float gravity_modifier;
        int max_particles;
        float playback_speed;
        bool local_space;
        Color start_color;
        ColorGradient start_color_gradient;
        float start_delay;
        float start_lifetime;
        float start_rotation;
        float start_size;
        float start_speed;
        float time;
        bool enable_emission;
        float emission_rate;
        ParticleEmitterShape::Enum emitter_shape;
        float emitter_shape_sphere_radius;
        float emitter_shape_cone_angle;
        float emitter_shape_cone_radius;
        EmitterShapeConeFrom::Enum emitter_shape_cone_from;
        Vector3 emitter_shape_box_size;
        bool emitter_random_direction;
        VelocityType::Enum velocity_type;
        Vector3 velocity;
        AnimationCurve velocity_curve_x;
        AnimationCurve velocity_curve_y;
        AnimationCurve velocity_curve_z;
        ForceType::Enum force_type;
        Vector3 force;
        AnimationCurve force_curve_x;
        AnimationCurve force_curve_y;
        AnimationCurve force_curve_z;
        ColorGradient color_gradient;
        AnimationCurve size_curve;
        float angular_velocity;
        bool enable_texture_sheet_animation;
        int texture_sheet_animation_tile_x;
        int texture_sheet_animation_tile_y;
        bool texture_sheet_animation_single_row;
        bool texture_sheet_animation_random_row;
        int texture_sheet_animation_row;
        AnimationCurve texture_sheet_animation_frame_curve;
        int texture_sheet_animation_cycles;

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
            emitter_random_direction(false),
            force_type(ForceType::Disable),
            angular_velocity(0),
            enable_texture_sheet_animation(false),
            m_time_emit(-1),
            m_vertex_buffer(NULL),
            m_index_buffer(NULL)
        {}
        virtual ~ParticleSystem();
        ID3D11Buffer *GetVertexBuffer();
        ID3D11Buffer *GetIndexBuffer();
        int GetParticleCount() const {return m_particles.size();}
        int GetParticleCountMax() const;
        void SetTargetCamera(const std::shared_ptr<Camera> &camera) {m_target_camera = camera;}

    protected:
        virtual void Awake();
        virtual void Update();

    private:
        std::list<Particle> m_particles;
        float m_time_emit;
        ID3D11Buffer *m_vertex_buffer;
        ID3D11Buffer *m_index_buffer;
        std::shared_ptr<Camera> m_target_camera;

        void UpdateEmitter();
        void EmitShapeSphere(Vector3 &position, Vector3 &velocity);
        void EmitShapeCone(Vector3 &position, Vector3 &velocity);
        void EmitShapeBox(Vector3 &position, Vector3 &velocity);
        void Emit(const Vector3 &position, const Vector3 &velocity, float size, float lifetime, const Color &color);
        void UpdateParticles();
        void UpdateBuffer();
    };
}

#endif
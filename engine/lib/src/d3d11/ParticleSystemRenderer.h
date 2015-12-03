#ifndef __ParticleSystemRenderer_h__
#define __ParticleSystemRenderer_h__

#include "Renderer.h"

namespace Galaxy3D
{
    class ParticleSystem;

    struct ParticleSystemRenderMode
    {
        enum Enum
        {
            Billboard,
            Stretch,
            //HorizontalBillboard,
            //VerticalBillboard,
            //Mesh,
        };
    };

    class ParticleSystemRenderer : public Renderer
    {
    public:
        std::weak_ptr<ParticleSystem> particle_system;
        ParticleSystemRenderMode::Enum render_mode;
        float stretch_speed_scale;
        float stretch_length_scale;

        ParticleSystemRenderer():
            render_mode(ParticleSystemRenderMode::Billboard)
        {
        }

    protected:
        virtual void Render(int material_index);
    };
}

#endif
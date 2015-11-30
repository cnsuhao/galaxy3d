#ifndef __ParticleSystemRenderer_h__
#define __ParticleSystemRenderer_h__

#include "Renderer.h"
#include "ParticleSystem.h"

namespace Galaxy3D
{
    class ParticleSystemRenderer : public Renderer
    {
    public:
        std::shared_ptr<ParticleSystem> particle_system;

    protected:
        virtual void Render(int material_index);
    };
}

#endif
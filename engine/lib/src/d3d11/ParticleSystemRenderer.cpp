#include "ParticleSystemRenderer.h"

namespace Galaxy3D
{
    void ParticleSystemRenderer::Render(int material_index)
    {
        auto vertex_buffer = particle_system->GetVertexBuffer();
        auto index_buffer = particle_system->GetIndexBuffer();
    }
}
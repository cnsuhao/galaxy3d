#ifndef __TerrainRenderer_h__
#define __TerrainRenderer_h__

#include "Renderer.h"
#include "Terrain.h"

namespace Galaxy3D
{
    class TerrainRenderer : public Renderer
    {
    protected:
        virtual void Start();
        virtual void Render(int material_index);

    private:
        std::shared_ptr<Terrain> m_terrain;
    };
}

#endif
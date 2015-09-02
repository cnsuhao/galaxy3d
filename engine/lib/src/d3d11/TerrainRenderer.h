#ifndef __TerrainRenderer_h__
#define __TerrainRenderer_h__

#include "Renderer.h"
#include "Terrain.h"

namespace Galaxy3D
{
    class TerrainRenderer : public Renderer
    {
    public:
        virtual void Render();
        void SetTerrain(const std::shared_ptr<Terrain> &terrain) {m_terrain = terrain;}

    private:
        std::shared_ptr<Terrain> m_terrain;
    };
}

#endif
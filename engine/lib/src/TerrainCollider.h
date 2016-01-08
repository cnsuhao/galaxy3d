#ifndef __TerrainCollider_h__
#define __TerrainCollider_h__

#include "Component.h"
#include "Terrain.h"

class btTriangleIndexVertexArray;

namespace Galaxy3D
{
    class TerrainCollider : public Component
    {
    public:
        TerrainCollider():
            m_collider_data(NULL)
        {}
        virtual ~TerrainCollider();
        virtual void Start();
        void SetTerrain(const std::shared_ptr<Terrain> &terrain) {m_terrain = terrain;}

    private:
        std::shared_ptr<Terrain> m_terrain;
        btTriangleIndexVertexArray *m_collider_data;
    };
}

#endif
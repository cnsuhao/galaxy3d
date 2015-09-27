#ifndef __Terrain_h__
#define __Terrain_h__

#include "Component.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Material.h"
#include "Camera.h"
#include "VertexType.h"
#include "FrustumBounds.h"

namespace Galaxy3D
{
	class Terrain : public Component
    {
    public:
	    struct IVec2
	    {
		    int x;
		    int y;

		    IVec2(int x, int y):x(x),y(y){}
	    };

	    struct GeoMipmapPatch
	    {
		    float delta;
		    int level;
		    bool visible;
		    Vector3 center;
		    float bounding_sphere_radius;

            GeoMipmapPatch() {}
	    };

	    struct GeoMipmapNeighbor
	    {
		    bool left;
		    bool top;
		    bool right;
		    bool bottom;
	    };

        Terrain();
	    ~Terrain();
	    void LoadData(
		    int size_heightmap,
            float unit_size,
            float unit_height,
		    const std::string &file_heightmap,
		    const std::string &file_alphamap,
		    const std::vector<std::string> &file_textures,
		    float texture_size);
        void SetCamera(const std::shared_ptr<Camera> &camera) {m_camera = camera;}
	    float GetHeight(const Vector3 &world_pos) const;
        std::shared_ptr<Material> GetSharedMaterial() const {return m_shared_material;}
        ID3D11Buffer *GetVertexBuffer();
        ID3D11Buffer *GetIndexBuffer();
        void UseGeoLod(bool use) {m_use_geo = use;}
        int GetIndexCount() const { return m_use_geo ? m_geo_indices.size() : m_indices.size();}
        const std::vector<VertexMesh> &GetVertices() const {return m_vertices;}
        const std::vector<int> &GetIndices() const {return m_indices;}

    private:
        int m_map_size;
        float m_xz_unit;
        std::vector<VertexMesh> m_vertices;
        std::vector<int> m_indices;
        ID3D11Buffer *m_vertex_buffer;
        ID3D11Buffer *m_index_buffer;
        std::shared_ptr<Material> m_shared_material;
        std::shared_ptr<Camera> m_camera;
        GeoMipmapPatch *m_geo_patches;
        int m_geo_patch_count_per_side;
        std::vector<int> m_geo_indices;
        bool m_use_geo;

        void UpdateIndexBuffer();
        void DeleteBuffers();
        void CalculateNormals();
        void UpdateGeoMipmap();
    };
}

#endif
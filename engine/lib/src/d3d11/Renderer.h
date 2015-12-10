#ifndef __Renderer_h__
#define __Renderer_h__

#include "Component.h"
#include "Vector4.h"
#include "Material.h"
#include "Camera.h"
#include "Transform.h"
#include "VertexType.h"
#include "Bounds.h"
#include "GTTime.h"
#include "FrustumBounds.h"
#include "GraphicsDevice.h"
#include <list>

namespace Galaxy3D
{
    class Octree;
    struct OctreeNode;
    class Renderer;

    struct RenderBatch
    {
        Renderer *renderer;
        int material_index;
        int static_batching_index_offset;
        int static_batching_index_count;

        RenderBatch():
            renderer(NULL),
            material_index(-1),
            static_batching_index_offset(-1),
            static_batching_index_count(0)
        {}
        bool IsStaticSinglePassMeshRenderer() const;
    };

	class Renderer : public Component
	{
	public:
		static void SortAllBatches();
		static void RenderAll();
        static void BuildOctree(const std::shared_ptr<GameObject> &obj);
        static void Init();
        static void Done();
        static void BuildStaticBatches();
        virtual ~Renderer();
		void SetVisible(bool visible) {m_visible = visible;}
		bool IsVisible() const {return m_visible;}
		void SetSortingOrder(int layer, int order);
		void SetSharedMaterials(const std::vector<std::shared_ptr<Material>> &materials, bool sort_renders = true);
		const std::vector<std::shared_ptr<Material>> &GetSharedMaterials() const {return m_shared_materials;}
		void SetSharedMaterial(const std::shared_ptr<Material> &material, bool sort_renders = true);
		std::shared_ptr<Material> GetSharedMaterial() const;
		std::vector<std::shared_ptr<Material>> GetMaterials() const;
		std::shared_ptr<Material> GetMaterial() const;
        void SetLightmapIndex(int lightmap_index) {m_lightmap_index = lightmap_index;}
        int GetLightmapIndex() const {return m_lightmap_index;}
        void SetLightmapTilingOffset(Vector4 lightmap_tiling_offset) {m_lightmap_tiling_offset = lightmap_tiling_offset;}
        void SetBounds(const Bounds &bounds) {m_bounds = bounds;}
        Bounds GetBounds() const {return m_bounds;}

	protected:
        static ID3D11Buffer *m_static_batching_vertex_buffer;
        static ID3D11Buffer *m_static_batching_index_buffer;
		bool m_cast_shadow;
		bool m_receive_shadow;
		bool m_visible;
		int m_lightmap_index;
		Vector4 m_lightmap_tiling_offset;
		int m_sorting_layer;
		int m_sorting_order;
        // aabb in world space
        Bounds m_bounds;

		Renderer();
        void DrawIndexed(int count, int offset);
		virtual void Render(int material_index) = 0;

	private:
        static std::list<RenderBatch> m_batches;
        static std::shared_ptr<Octree> m_octree;
		std::vector<std::shared_ptr<Material>> m_shared_materials;

        static void SortTransparentBatches();
        static bool LessBatch(const RenderBatch &b1, const RenderBatch &b2);
        static void ViewFrustumCulling(const FrustumBounds &frustum, const std::shared_ptr<OctreeNode> &node);
        void AddBatches();
        void RemoveBatches();
	};
}

#endif
#ifndef __Renderer_h__
#define __Renderer_h__

#include "Component.h"
#include "Vector4.h"
#include "Material.h"
#include "Camera.h"
#include "Transform.h"
#include "VertexType.h"
#include "GTTime.h"
#include <list>

namespace Galaxy3D
{
	class Renderer : public Component
	{
	public:
		static void Sort();
		static void RenderAll();
        virtual ~Renderer();
		void SetVisible(bool visible) {m_visible = visible;}
		bool IsVisible() const {return m_visible;}
		void SetSortingOrder(int layer, int order);
		void SetSharedMaterials(const std::vector<std::shared_ptr<Material>> &materials);
		const std::vector<std::shared_ptr<Material>> &GetSharedMaterials() const {return m_shared_materials;}
		void SetSharedMaterial(const std::shared_ptr<Material> &material);
		std::shared_ptr<Material> GetSharedMaterial() const;
		std::vector<std::shared_ptr<Material>> GetMaterials() const;
		std::shared_ptr<Material> GetMaterial() const;
        void SetLightmapIndex(int lightmap_index) {m_lightmap_index = lightmap_index;}
        void SetLightmapTilingOffset(Vector4 lightmap_tiling_offset) {m_lightmap_tiling_offset = lightmap_tiling_offset;}

	protected:
		bool m_cast_shadow;
		bool m_receive_shadow;
		bool m_visible;
		int m_lightmap_index;
		Vector4 m_lightmap_tiling_offset;
		int m_sorting_layer;
		int m_sorting_order;

		Renderer();
        void DrawIndexed(int count, int offset);
		virtual void Render() = 0;

	private:
		static std::list<Renderer *> m_renderers;
		std::vector<std::shared_ptr<Material>> m_shared_materials;

		static bool Less(const Renderer *c1, const Renderer *c2);
	};
}

#endif
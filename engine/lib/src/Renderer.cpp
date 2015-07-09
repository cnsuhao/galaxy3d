#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "LayerMask.h"

namespace Galaxy3D
{
	std::list<Renderer *> Renderer::m_renderers;

	Renderer::Renderer():
		m_cast_shadow(false),
		m_receive_shadow(false),
		m_visible(true),
		m_lightmap_index(-1),
		m_lightmap_tiling_offset(),
		m_sorting_layer(-1),
		m_sorting_order(-1)
	{
		m_renderers.push_back(this);
	}

	Renderer::~Renderer()
	{
		m_renderers.remove(this);
	}

	void Renderer::SetSortingLayer(int layer)
	{
		m_sorting_layer = layer;
		Sort();
	}

	void Renderer::SetSortingOrder(int order)
	{
		m_sorting_order = order;
		Sort();
	}

	void Renderer::Sort()
	{
		m_renderers.sort(Less);
	}

	bool Renderer::Less(const Renderer *c1, const Renderer *c2)
	{
		auto m1 = c1->GetSharedMaterial();
		auto m2 = c2->GetSharedMaterial();

		if(!m1 || !m2)
		{
			return false;
		}

		int q1 = m1->GetRenderQueue();
		if(q1 < 0)
		{
			q1 = m1->GetShader()->GetRenderQueue();
		}

		int q2 = m2->GetRenderQueue();
		if(q2 < 0)
		{
			q2 = m2->GetShader()->GetRenderQueue();
		}

		if(q1 == q2)
		{
			if(c1->m_sorting_layer >= 0 && c2->m_sorting_layer >= 0)
			{
				if(c1->m_sorting_layer == c2->m_sorting_layer)
				{
					return c1->m_sorting_order < c2->m_sorting_order;
				}
				else
				{
					return c1->m_sorting_layer < c2->m_sorting_layer;
				}
			}
		}
		
		return q1 < q2;
	}

	void Renderer::SetSharedMaterials(const std::vector<std::shared_ptr<Material>> &materials)
	{
		m_shared_materials = materials;

		Sort();
	}

	void Renderer::SetSharedMaterial(const std::shared_ptr<Material> &material)
	{
		m_shared_materials.clear();

		if(material)
		{
			m_shared_materials.push_back(material);
		}

		Sort();
	}

	std::shared_ptr<Material> Renderer::GetSharedMaterial() const
	{
		std::shared_ptr<Material> mat;

		if(!m_shared_materials.empty())
		{
			mat = m_shared_materials.front();
		}

		return mat;
	}

	std::vector<std::shared_ptr<Material>> Renderer::GetMaterials() const
	{
		std::vector<std::shared_ptr<Material>> mats(m_shared_materials.size());

		for(size_t i=0; i<mats.size(); i++)
		{
			auto mat = Material::Create(m_shared_materials[i]->GetShader());
			mat->Copy(*m_shared_materials[i].get());

			mats[i] = mat;
		}

		return mats;
	}

	std::shared_ptr<Material> Renderer::GetMaterial() const
	{
		std::shared_ptr<Material> mat;

		auto shared = GetSharedMaterial();
		if(shared)
		{
			mat = Material::Create(shared->GetShader());
			mat->Copy(*shared.get());
		}

		return mat;
	}

	void Renderer::RenderAll()
	{
		auto camera = Camera::GetCurrent();

		GTTime::m_draw_call = 0;
		for(auto i : m_renderers)
		{
			auto obj = i->GetGameObject();

			if(	obj->IsActiveInHierarchy() &&
				i->IsEnable() &&
				i->IsVisible() &&
				((camera->GetCullingMask() & LayerMask::GetMask(obj->GetLayer())) != 0))
			{
				i->Render();
				GTTime::m_draw_call++;
			}
		}
	}
}
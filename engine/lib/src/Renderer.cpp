#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "LayerMask.h"
#include "Octree.h"
#include "FrustumBounds.h"

namespace Galaxy3D
{
    std::list<RenderBatch> Renderer::m_batches;
    std::shared_ptr<Octree> Renderer::m_octree;

	Renderer::Renderer():
		m_cast_shadow(false),
		m_receive_shadow(false),
		m_visible(true),
		m_lightmap_index(-1),
		m_lightmap_tiling_offset(),
		m_sorting_layer(-1),
		m_sorting_order(-1),
        m_bounds(Vector3(0, 0, 0), Vector3(1, 1, 1) * Mathf::MaxFloatValue)
	{
	}

	Renderer::~Renderer()
	{
        RemoveBatches();
	}

	void Renderer::SetSortingOrder(int layer, int order)
	{
		m_sorting_layer = layer;
		m_sorting_order = order;

        Sort();
	}

	void Renderer::Sort()
	{
        m_batches.sort(LessBatch);
	}

	bool Renderer::Less(const Renderer *c1, const Renderer *c2)
	{
		auto &m1 = c1->GetSharedMaterial();
		auto &m2 = c2->GetSharedMaterial();

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

    bool Renderer::LessBatch(const RenderBatch &b1, const RenderBatch &b2)
    {
        auto &m1 = b1.renderer->GetSharedMaterials()[b1.material_index];
        auto &m2 = b2.renderer->GetSharedMaterials()[b2.material_index];

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
            if(b1.renderer->m_sorting_layer >= 0 && b2.renderer->m_sorting_layer >= 0)
            {
                if(b1.renderer->m_sorting_layer == b2.renderer->m_sorting_layer)
                {
                    return b1.renderer->m_sorting_order < b2.renderer->m_sorting_order;
                }
                else
                {
                    return b1.renderer->m_sorting_layer < b2.renderer->m_sorting_layer;
                }
            }
        }

        return q1 < q2;
    }

    void Renderer::AddBatches()
    {
        for(size_t i=0; i<m_shared_materials.size(); i++)
        {
            RenderBatch batch;
            batch.renderer = this;
            batch.material_index = i;

            m_batches.push_back(batch);
        }
    }

    void Renderer::RemoveBatches()
    {
        for(auto i=m_batches.begin(); i!=m_batches.end(); )
        {
            if(i->renderer == this)
            {
                i = m_batches.erase(i);
            }
            else
            {
                i++;
            }
        }
    }

	void Renderer::SetSharedMaterials(const std::vector<std::shared_ptr<Material>> &materials)
	{
        RemoveBatches();

		m_shared_materials = materials;

        AddBatches();

        Sort();
	}

	void Renderer::SetSharedMaterial(const std::shared_ptr<Material> &material)
	{
        RemoveBatches();

		m_shared_materials.clear();

		if(material)
		{
			m_shared_materials.push_back(material);

            AddBatches();
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

    void Renderer::ViewFrustumCulling(const FrustumBounds &frustum, const std::shared_ptr<OctreeNode> &node)
    {
        auto camera = Camera::GetCurrent();

        auto contains = frustum.ContainsBounds(node->center, node->extents);
        if(contains == ContainsResult::Out)
        {
            // all renderers in this node are invisible
            node->SetVisible(false);
        }
        else
        {
            for(size_t i=0; i<node->renderers.size(); i++)
            {
                auto &renderer = node->renderers[i].lock();
                auto obj = renderer->GetGameObject();

                if(	obj->IsActiveInHierarchy() &&
                    renderer->IsEnable() &&
                    ((camera->GetCullingMask() & LayerMask::GetMask(obj->GetLayer())) != 0))
                {
                    auto bounds = renderer->GetBounds();
                    contains = frustum.ContainsBounds(bounds.center, bounds.extents);
                    if(contains == ContainsResult::Out)
                    {
                        renderer->SetVisible(false);
                    }
                    else
                    {
                        renderer->SetVisible(true);
                    }
                }
                else
                {
                    renderer->SetVisible(false);
                }
            }

            for(int i=0; i<8; i++)
            {
                auto &child = node->children[i];

                if(child)
                {
                    ViewFrustumCulling(frustum, child);
                }
            }
        }
    }

	void Renderer::RenderAll()
	{
        // sort all batches every frame
        //Sort();

		auto camera = Camera::GetCurrent();

        if(m_octree)
        {
            FrustumBounds frustum(camera->GetViewProjectionMatrix());
            ViewFrustumCulling(frustum, m_octree->GetRootNode());
        }

        for(auto &i : m_batches)
        {
            auto obj = i.renderer->GetGameObject();

            // if obj is inactive, 
            // or renderer is disabled,
            // or obj culling by layer,
            // it will be invisible.
            if(i.renderer->IsVisible())
            {
                i.renderer->Render(i.material_index);
            }
        }
	}

    void Renderer::DrawIndexed(int count, int offset)
    {
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
        context->DrawIndexed(count, offset, 0);

        GTTime::m_draw_call++;
    }

    void Renderer::BuildOctree(const std::shared_ptr<GameObject> &obj)
    {
        m_octree = std::shared_ptr<Octree>(new Octree(obj));
    }
}
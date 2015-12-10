#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "LayerMask.h"
#include "Octree.h"
#include "FrustumBounds.h"
#include "Debug.h"
#include "MeshRenderer.h"

namespace Galaxy3D
{
    static const int TRANSPARENT_ORDER_MIN = 2500;
    std::list<RenderBatch> Renderer::m_batches;
    std::list<RenderBatch> Renderer::m_batches_for_render;
    std::shared_ptr<Octree> Renderer::m_octree;
    ID3D11Buffer *Renderer::m_static_batching_vertex_buffer = NULL;
    ID3D11Buffer *Renderer::m_static_batching_index_buffer = NULL;

    bool RenderBatch::IsStaticSinglePassMeshRenderer() const
    {
        auto obj = renderer->GetGameObject();
        auto mesh_renderer = dynamic_cast<MeshRenderer *>(renderer);
        bool single_pass = renderer->GetSharedMaterials()[material_index]->GetShader()->GetPassCount() == 1;

        return obj->IsStatic() && mesh_renderer != NULL && single_pass;
    }

	Renderer::Renderer():
		m_cast_shadow(false),
		m_receive_shadow(false),
		m_visible(true),
		m_lightmap_index(-1),
		m_lightmap_tiling_offset(),
		m_sorting_layer(0),
		m_sorting_order(0),
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

        Renderer::SortAllBatches();
	}

	void Renderer::SortAllBatches()
	{
        m_batches.sort(LessBatch);
	}

    void Renderer::SortTransparentBatches()
    {
        // find first transparent renderer
        auto i = m_batches_for_render.begin();
        for(; i != m_batches_for_render.end(); i++)
        {
            if(i->renderer->GetSharedMaterials()[i->material_index]->GetRenderQueue() > TRANSPARENT_ORDER_MIN)
            {
                break;
            }
        }

        // splice to temp list for sorting by distance
        std::list<RenderBatch> transparents;
        transparents.splice(transparents.begin(), m_batches_for_render, i, m_batches_for_render.end());

        transparents.sort(LessBatch);

        // splice back
        m_batches_for_render.splice(m_batches_for_render.end(), transparents);
    }

    bool Renderer::LessBatch(const RenderBatch &b1, const RenderBatch &b2)
    {
        int compare;

        auto &m1 = b1.renderer->GetSharedMaterials()[b1.material_index];
        auto &m2 = b2.renderer->GetSharedMaterials()[b2.material_index];

        int q1 = m1->GetRenderQueue();
        int q2 = m2->GetRenderQueue();

        compare = q1 - q2;

        if(compare == 0)
        {
            if(b1.renderer->m_sorting_layer == b2.renderer->m_sorting_layer)
            {
                compare = b1.renderer->m_sorting_order - b2.renderer->m_sorting_order;
            }
            else
            {
                compare = b1.renderer->m_sorting_layer - b2.renderer->m_sorting_layer;
            }
        }

        if(compare == 0)
        {
            if(q1 > TRANSPARENT_ORDER_MIN)
            {
                // order by distance
                auto cam = Camera::GetCurrent();
                if(cam)
                {
                    float dis1 = (cam->GetTransform()->GetPosition() - b1.renderer->GetTransform()->GetPosition()).SqrMagnitude();
                    float dis2 = (cam->GetTransform()->GetPosition() - b2.renderer->GetTransform()->GetPosition()).SqrMagnitude();
                    
                    if(dis1 < dis2)
                    {
                        compare = -1;
                    }
                    else if(dis1 > dis2)
                    {
                        compare = 1;
                    }
                }
            }
            else
            {
                compare = m1->GetGuid().compare(m2->GetGuid());

                if(compare == 0)
                {
                    compare = b1.renderer->m_lightmap_index - b2.renderer->m_lightmap_index;
                }

                if(compare == 0)
                {
                    int is_static_single_pass_mesh_renderer_1 = b1.IsStaticSinglePassMeshRenderer() ? 1 : 0;
                    int is_static_single_pass_mesh_renderer_2 = b2.IsStaticSinglePassMeshRenderer() ? 1 : 0;

                    compare = is_static_single_pass_mesh_renderer_1 - is_static_single_pass_mesh_renderer_2;
                }
            }
        }

        return compare < 0;
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

	void Renderer::SetSharedMaterials(const std::vector<std::shared_ptr<Material>> &materials, bool sort_renders)
	{
        RemoveBatches();

		m_shared_materials = materials;

        AddBatches();

        if(sort_renders)
        {
            Renderer::SortAllBatches();
        }
	}

	void Renderer::SetSharedMaterial(const std::shared_ptr<Material> &material, bool sort_renders)
	{
        RemoveBatches();

		m_shared_materials.clear();

		if(material)
		{
			m_shared_materials.push_back(material);

            AddBatches();
		}

        if(sort_renders)
        {
            Renderer::SortAllBatches();
        }
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

    void Renderer::UpdateForRender()
    {
        m_batches_for_render = m_batches;
    }

	void Renderer::RenderAll()
	{
        // sort all transparent batches every frame
        SortTransparentBatches();
        
        auto camera = Camera::GetCurrent();

        if(m_octree)
        {
            FrustumBounds frustum(camera->GetViewProjectionMatrix());
            ViewFrustumCulling(frustum, m_octree->GetRootNode());
        }

        RenderBatch *last_batch = NULL;
        std::list<RenderBatch *> dynamic_batches;
        for(auto i=m_batches_for_render.begin(); i!=m_batches_for_render.end(); i++)
        {
            auto obj = i->renderer->GetGameObject();

            if( i->renderer->IsVisible() &&
                obj->IsActiveInHierarchy() &&
                i->renderer->IsEnable() &&
                ((camera->GetCullingMask() & LayerMask::GetMask(obj->GetLayer())) != 0))
            {
                auto mesh_renderer = dynamic_cast<MeshRenderer *>(i->renderer);

                if( i->IsStaticSinglePassMeshRenderer() &&
                    m_static_batching_vertex_buffer != NULL &&
                    m_static_batching_index_buffer != NULL)
                {
                    mesh_renderer->RenderStaticBatch(&(*i), last_batch);
                }
                else
                {
                    i->renderer->Render(i->material_index);
                }

                last_batch = &(*i);
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

    void Renderer::Init()
    {
    }

    void Renderer::Done()
    {
        SAFE_RELEASE(m_static_batching_vertex_buffer);
        SAFE_RELEASE(m_static_batching_index_buffer);
    }

    void Renderer::BuildStaticBatches()
    {
        auto vertices = new std::vector<VertexMesh>();
        auto indices = new std::vector<unsigned int>();

        for(auto &i : m_batches)
        {
            auto mesh_renderer = dynamic_cast<MeshRenderer *>(i.renderer);

            if(i.IsStaticSinglePassMeshRenderer())
            {
                auto mesh = mesh_renderer->GetMesh();
                auto &vs = mesh->GetVertices();
                auto &is = mesh->GetIndices();
                auto &mat = i.renderer->GetTransform()->GetLocalToWorldMatrix();

                int vertices_size_old = vertices->size();
                size_t vertex_count = vs.size();
                for(size_t j=0; j<vertex_count; j++)
                {
                    VertexMesh &v_model = vs[j];
                    VertexMesh v = v_model;
                    v.POSITION = mat.MultiplyPoint3x4(v_model.POSITION);
                    v.NORMAL = mat.MultiplyPoint3x4(v_model.NORMAL);
                    v.TANGENT = mat * v_model.TANGENT;

                    vertices->push_back(v);
                }

                int indices_size_old = indices->size();
                auto &is_submesh = is[i.material_index];
                size_t index_count = is_submesh.size();
                for(size_t j=0; j<index_count; j++)
                {
                    unsigned int index = vertices_size_old + is_submesh[j];

                    indices->push_back(index);
                }

                i.static_batching_index_offset = indices_size_old;
                i.static_batching_index_count = index_count;
            }
        }

        SAFE_RELEASE(m_static_batching_vertex_buffer);
        SAFE_RELEASE(m_static_batching_index_buffer);

        if(!vertices->empty() && !indices->empty())
        {
            auto device = GraphicsDevice::GetInstance()->GetDevice();

            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.CPUAccessFlags = 0;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.ByteWidth = sizeof(VertexMesh) * vertices->size();

            D3D11_SUBRESOURCE_DATA data;
            ZeroMemory(&data, sizeof(data));
            data.pSysMem = &(*vertices)[0];
            HRESULT hr = device->CreateBuffer(&bd, &data, &m_static_batching_vertex_buffer);

            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.CPUAccessFlags = 0;
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.ByteWidth = sizeof(unsigned int) * indices->size();

            ZeroMemory(&data, sizeof(data));
            data.pSysMem = &(*indices)[0];
            hr = device->CreateBuffer(&bd, &data, &m_static_batching_index_buffer);
        }

        delete vertices;
        delete indices;
    }
}
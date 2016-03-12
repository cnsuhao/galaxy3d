#include "Renderer.h"
#include "GameObject.h"
#include "Camera.h"
#include "Octree.h"
#include "FrustumBounds.h"
#include "Debug.h"
#include "MeshRenderer.h"
#include "RenderSettings.h"

namespace Galaxy3D
{
    std::list<RenderBatch> Renderer::m_batches;
    std::list<RenderBatch> Renderer::m_batches_renderable_opaque;
    std::list<RenderBatch> Renderer::m_batches_renderable_transparent;
    std::shared_ptr<Octree> Renderer::m_octree;
    BufferObject Renderer::m_static_batching_vertex_buffer;
    BufferObject Renderer::m_static_batching_index_buffer;

    bool RenderBatch::IsStaticSinglePassMeshRenderer() const
    {
        auto obj = renderer->GetGameObject();
        auto mesh_renderer = dynamic_cast<MeshRenderer *>(renderer);
        auto shader = renderer->GetSharedMaterials()[material_index]->GetShader();
        int pass_count = shader->GetPassCount();
        int count = 0;
        for(int i=0; i<pass_count; i++)
        {
            auto name = shader->GetPass(i)->name;
            if(name != "deferred" && name != "depth")
            {
                count++;
            }
        }

        return obj->IsStatic() && mesh_renderer != NULL && count == 1;
    }

	Renderer::Renderer():
		m_cast_shadow(true),
		m_receive_shadow(true),
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

    void Renderer::DeepCopy(const std::shared_ptr<Object> &source)
    {
        auto src_renderer = std::dynamic_pointer_cast<Renderer>(source);

        Component::DeepCopy(source);

        m_cast_shadow = src_renderer->m_cast_shadow;
        m_receive_shadow = src_renderer->m_receive_shadow;
        m_visible = src_renderer->m_visible;
        m_lightmap_index = src_renderer->m_lightmap_index;
        m_lightmap_tiling_offset = src_renderer->m_lightmap_tiling_offset;
        m_sorting_layer = src_renderer->m_sorting_layer;
        m_sorting_order = src_renderer->m_sorting_order;
        m_bounds = src_renderer->m_bounds;

        SetSharedMaterials(src_renderer->m_shared_materials);
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
                        compare = 1;
                    }
                    else if(dis1 > dis2)
                    {
                        compare = -1;
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
                auto renderer = node->renderers[i].lock();
                auto obj = renderer->GetGameObject();

                if(	obj->IsActiveInHierarchy() &&
                    renderer->IsEnable() &&
                    !camera->IsCulling(obj))
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

    void Renderer::Prepare()
    {
        auto camera = Camera::GetCurrent();

        m_batches_renderable_opaque.clear();
        m_batches_renderable_transparent.clear();
        for(auto i=m_batches.begin(); i!=m_batches.end(); i++)
        {
            auto obj = i->renderer->GetGameObject();

            if( obj->IsActiveInHierarchy() &&
                i->renderer->IsEnable() &&
                !camera->IsCulling(obj))
            {
                if(i->renderer->GetSharedMaterials()[i->material_index]->GetRenderQueue() <= TRANSPARENT_ORDER_MIN)
                {
                    m_batches_renderable_opaque.push_back(*i);
                }
                else
                {
                    m_batches_renderable_transparent.push_back(*i);
                }
            }
        }

        // sort all transparent batches every frame
        m_batches_renderable_transparent.sort(LessBatch);

        if(m_octree)
        {
            FrustumBounds frustum(camera->GetViewProjectionMatrix());
            ViewFrustumCulling(frustum, m_octree->GetRootNode());
        }
    }

    const std::list<RenderBatch> &Renderer::GetRenderBatches()
    {
        return m_batches;
    }

    void Renderer::RenderOpaqueGeometry()
    {
        auto shadow_light = RenderSettings::GetLightRenderingShadowMap();
        if(shadow_light)
        {
            std::list<RenderBatch> batches;
            auto type = shadow_light->GetType();

            if(type == LightType::Spot)
            {
                FrustumBounds frustum(shadow_light->GetViewProjectionMatrix());

                for(auto &i : m_batches_renderable_opaque)
                {
                    auto bounds = i.renderer->GetBounds();
                    int contains = frustum.ContainsBounds(bounds.center, bounds.extents);
                    if( contains != ContainsResult::Out &&
                        i.renderer->IsCastShadow())
                    {
                        batches.push_back(i);
                    }
                }
            }
            else if(type == LightType::Directional)
            {
                auto &m = shadow_light->GetProjectionMatrix();
                float right = (1 - m.m03) / m.m00;
                float left = (-1 - m.m03) / m.m00;
                float top = (1 - m.m13) / m.m11;
                float bottom = (-1 - m.m13) / m.m11;
                float z_near = m.m23 / m.m22;
                float z_far = (m.m23 - 1) / m.m22;

                auto frustum = FrustumBounds::FrustumBoundsOrtho(left, right, bottom, top, z_near, z_far);
                auto &world_to_light = shadow_light->GetWorldToLocalMatrix();

                auto camera = Camera::GetCurrent();
                for(auto &i : m_batches)
                {
                    auto obj = i.renderer->GetGameObject();
                    if( !(obj->IsActiveInHierarchy() &&
                        i.renderer->IsEnable() &&
                        !camera->IsCulling(obj) &&
                        i.renderer->GetSharedMaterials()[i.material_index]->GetRenderQueue() <= TRANSPARENT_ORDER_MIN &&
                        i.renderer->IsCastShadow()))
                    {
                        continue;
                    }

                    auto bounds = i.renderer->GetBounds();

                    int contains;
                    
                    if( Mathf::FloatEqual(bounds.extents.x, Mathf::MaxFloatValue) ||
                        Mathf::FloatEqual(bounds.extents.y, Mathf::MaxFloatValue) ||
                        Mathf::FloatEqual(bounds.extents.z, Mathf::MaxFloatValue))
                    {
                        contains = ContainsResult::Cross;
                    }
                    else
                    {
                        int j = 0;
                        std::vector<Vector3> corners(8);
                        corners[j++] = bounds.center + Vector3(-bounds.extents.x, bounds.extents.y, -bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(-bounds.extents.x, -bounds.extents.y, -bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(bounds.extents.x, -bounds.extents.y, -bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(bounds.extents.x, bounds.extents.y, -bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(-bounds.extents.x, bounds.extents.y, bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(-bounds.extents.x, -bounds.extents.y, bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(bounds.extents.x, -bounds.extents.y, bounds.extents.z);
                        corners[j++] = bounds.center + Vector3(bounds.extents.x, bounds.extents.y, bounds.extents.z);

                        contains = frustum.ContainsPoints(corners, &world_to_light);
                    }
                    
                    if(contains != ContainsResult::Out)
                    {
                        batches.push_back(i);
                    }
                }
            }
            
            RenderBatches(batches, true);
        }
        else
        {
            RenderBatches(m_batches_renderable_opaque);
        }
    }

    void Renderer::RenderTransparentGeometry()
    {
        RenderBatches(m_batches_renderable_transparent);
    }

    void Renderer::RenderBatches(const std::list<RenderBatch> &batches, bool ignore_frustum_culling)
    {
        const RenderBatch *last_batch = NULL;
        std::list<RenderBatch *> dynamic_batches;
        for(auto i=batches.begin(); i!=batches.end(); i++)
        {
            if(ignore_frustum_culling || i->renderer->IsVisible())
            {
                if( i->IsStaticSinglePassMeshRenderer() &&
                    m_static_batching_vertex_buffer.buffer != NULL &&
                    m_static_batching_index_buffer.buffer != NULL)
                {
                    auto mesh_renderer = dynamic_cast<MeshRenderer *>(i->renderer);
                    mesh_renderer->RenderStaticBatch(&(*i), last_batch);
                }
                else
                {
                    auto obj = i->renderer->GetGameObject();
                    obj->OnWillRenderObject(i->material_index);
                    i->renderer->Render(i->material_index);
                    obj->OnDidRenderObject(i->material_index);
                }

                last_batch = &(*i);
            }
        }
    }

    void Renderer::DrawIndexed(int count, int offset)
    {
        GraphicsDevice::GetInstance()->DrawIndexed(count, offset);

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
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_static_batching_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_static_batching_index_buffer);

        m_octree.reset();
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
                if(!mesh)
                {
                    continue;
                }

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
                    v.NORMAL = Vector3::Normalize(mat.MultiplyDirection(v_model.NORMAL));
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

        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_static_batching_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_static_batching_index_buffer);

        if(!vertices->empty() && !indices->empty())
        {
            m_static_batching_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(&(*vertices)[0], sizeof(VertexMesh) * vertices->size(), BufferUsage::StaticDraw, BufferType::Vertex);
            m_static_batching_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(&(*indices)[0], sizeof(unsigned int) * indices->size(), BufferUsage::StaticDraw, BufferType::Index);
        }

        delete vertices;
        delete indices;
    }
}
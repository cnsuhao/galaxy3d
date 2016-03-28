#include "LabelBatchRenderer.h"
#include "GameObject.h"
#include "SpriteBatchRenderer.h"

namespace Galaxy3D
{
	LabelBatchRenderer::LabelBatchRenderer():
		m_color(1, 1, 1, 1),
		m_clip(false),
        m_clip_rect(),
        m_clip_soft(),
        m_dirty(true),
        m_vertex_count_old(-1),
        m_index_count_old(-1)
	{
		m_sorting_layer = 0;
		m_sorting_order = 0;

		auto mat = Material::Create("UI/Text");
		SetSharedMaterial(mat);
	}

	LabelBatchRenderer::~LabelBatchRenderer()
	{
		Release();
	}

	void LabelBatchRenderer::Release()
	{
		GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
	}

	void LabelBatchRenderer::AddLabel(const std::shared_ptr<LabelNode> &label)
	{
		m_labels.push_back(label);
		m_labels.sort(LabelNode::Less);

        label->SetBatch(std::dynamic_pointer_cast<LabelBatchRenderer>(GetComponentPtr()));

        m_dirty = true;
	}

	void LabelBatchRenderer::RemoveLabel(const std::shared_ptr<LabelNode> &label)
	{
		for(auto i=m_labels.begin(); i!=m_labels.end(); i++)
		{
			if((*i) == label)
			{
				m_labels.erase(i);

                m_dirty = true;
				break;
			}
		}
	}

	bool LabelBatchRenderer::IsDirty()
	{
		bool any_label_dirty = false;

        for(auto &i : m_labels)
        {
            if(i->IsDirty() || i->GetLabel()->IsDirty())
            {
                any_label_dirty = true;
                break;
            }
        }

        return any_label_dirty || m_dirty;
	}

	static int get_labels_vertex_count(const std::list<std::shared_ptr<LabelNode>> &list)
    {
        int vertex_count = 0;
        for(auto &i : list)
        {
            vertex_count += i->GetLabel()->GetVertexCount();
        }

        return vertex_count;
    }

	static int get_labels_index_count(const std::list<std::shared_ptr<LabelNode>> &list)
    {
        int index_count = 0;
        for(auto &i : list)
        {
            if(i->GetGameObject()->IsActiveInHierarchy() && i->IsEnable())
            {
                index_count += i->GetLabel()->GetIndexCount();
            }
        }

        return index_count;
    }

	void LabelBatchRenderer::UpdateLabels()
	{
		int vertex_count = get_labels_vertex_count(m_labels);
        int index_count = get_labels_index_count(m_labels);

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
		{
            UpdateVertexBuffer(true);
            UpdateIndexBuffer(true);
		}
		else
		{
            if( m_vertex_count_old != vertex_count ||
                m_index_count_old != index_count)
			{
				Release();

                UpdateVertexBuffer(true);
                UpdateIndexBuffer(true);
			}
			else
			{
				UpdateVertexBuffer(false);
                UpdateIndexBuffer(false);
			}
		}

        m_vertex_count_old = vertex_count;
        m_index_count_old = index_count;
	}

	void LabelBatchRenderer::UpdateVertexBuffer(bool create)
	{
		int vertex_count = get_labels_vertex_count(m_labels);
        if(vertex_count)
        {
            int buffer_size = sizeof(VertexUI) * vertex_count;
            char *buffer = (char *) malloc(buffer_size);

            char *p = buffer;
            for(auto &i : m_labels)
            {
				auto local_position = GetTransform()->InverseTransformPoint(i->GetTransform()->GetPosition());
				auto local_rotation = Quaternion::Inverse(GetTransform()->GetRotation()) * i->GetTransform()->GetRotation();
				const Vector3 &parent_scale = GetTransform()->GetScale();
				Vector3 scale = i->GetTransform()->GetScale();
				float x = scale.x / parent_scale.x;
				float y = scale.y / parent_scale.y;
				float z = scale.z / parent_scale.z;
				auto local_scale = Vector3(x, y, z);
				auto matrix = Matrix4x4::TRS(local_position, local_rotation, local_scale);

				p += Label::FillVertexBuffer(p, i->GetLabel(), &matrix);
            }

            if(create)
            {
                m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Vertex);
            }
            else
            {
                GraphicsDevice::GetInstance()->UpdateBufferObject(m_vertex_buffer, buffer, buffer_size);
            }

            free(buffer);
        }
	}

	void LabelBatchRenderer::UpdateIndexBuffer(bool create)
	{
		int index_count = get_labels_index_count(m_labels);
        if(index_count > 0)
        {
            int buffer_size = sizeof(unsigned short) * index_count;
            char *buffer = (char *) malloc(buffer_size);
            char *p = buffer;

            int vertex_count = 0;
            for(auto &i : m_labels)
            {
				if(i->GetGameObject()->IsActiveInHierarchy() && i->IsEnable())
				{
					int count = i->GetLabel()->GetIndexCount();
					int size = Label::FillIndexBuffer(p, i->GetLabel());
					unsigned short *indices = (unsigned short *) p;

					for(int j=0; j<count; j++)
					{
                        indices[j] += vertex_count;
					}

					p += size;
				}

                vertex_count += i->GetLabel()->GetVertexCount();
            }

            if(create)
            {
                m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::DynamicDraw, BufferType::Index);
            }
            else
            {
                GraphicsDevice::GetInstance()->UpdateBufferObject(m_index_buffer, buffer, buffer_size);
            }

            free(buffer);
        }
	}

	void LabelBatchRenderer::Render(int material_index)
	{
		if(m_labels.empty())
		{
			return;
		}

		if(IsDirty())
        {
            m_dirty = false;
            for(auto &i : m_labels)
            {
                i->SetDirty(false);
				i->GetLabel()->SetDirty(false);
            }

            UpdateLabels();
        }

		if(m_vertex_buffer.buffer == NULL || m_index_buffer.buffer == NULL)
		{
			return;
		}

		auto mat = GetSharedMaterial();
		auto shader = mat->GetShader();
		auto pass = shader->GetPass(0);

        GraphicsDevice::GetInstance()->SetInputLayout(pass->vs);
        GraphicsDevice::GetInstance()->SetVertexBuffer(m_vertex_buffer, pass->vs);
        GraphicsDevice::GetInstance()->SetIndexBuffer(m_index_buffer, IndexType::UShort);
		
		auto camera = Camera::GetCurrent();
        Matrix4x4 wvp = camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();

		mat->SetMatrix("WorldViewProjection", wvp);
		mat->SetMainTexture(Label::GetFontTexture());
        mat->SetMainColor(m_color);
        if(m_clip && !m_clip_panel.expired())
        {
            auto wvp_clip = camera->GetViewProjectionMatrix() * m_clip_panel.lock()->GetTransform()->GetLocalToWorldMatrix();
            Vector3 min = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.x, m_clip_rect.w, 0));
            Vector3 max = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.z, m_clip_rect.y, 0));
            Vector4 rect(min.x, max.y, max.x, min.y);
            min = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.x + m_clip_soft.x, m_clip_rect.w + m_clip_soft.y, 0));
            max = wvp_clip.MultiplyPoint(Vector3(m_clip_rect.z - m_clip_soft.x, m_clip_rect.y - m_clip_soft.y, 0));
            Vector4 soft(min.x, max.y, max.x, min.y);
            mat->SetVector("ClipRect", rect);
            mat->SetVector("ClipSoft", soft);
        }
        else
        {
            mat->SetVector("ClipRect", Vector4(-1, 1, 1, -1));
            mat->SetVector("ClipSoft", Vector4(-1, 1, 1, -1));
        }
		
		mat->ReadyPass(0);
		pass->rs->Apply();
		mat->ApplyPass(0);

        DrawIndexed(get_labels_index_count(m_labels), 0);

		GraphicsDevice::GetInstance()->ClearShaderResources();
	}
}
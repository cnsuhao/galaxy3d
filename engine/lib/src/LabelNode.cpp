#include "LabelNode.h"
#include "GameObject.h"
#include "UICanvas.h"
#include "LabelBatchRenderer.h"

namespace Galaxy3D
{
	DEFINE_COM_CLASS(LabelNode);

	void LabelNode::DeepCopy(const std::shared_ptr<Object> &source)
	{
		auto src_node = std::dynamic_pointer_cast<LabelNode>(source);

		Component::DeepCopy(source);

		m_label = src_node->m_label;
		m_color = src_node->m_color;
		m_anchor = src_node->m_anchor;
		m_sorting_order = src_node->m_sorting_order;
		m_dirty = true;
	}

	void LabelNode::Start()
    {
        auto canvas = GetGameObject()->GetComponentInParent<UICanvas>();

        if(canvas && m_anchor)
        {
			auto t = GetTransform();
            canvas->AnchorTransform(t, *m_anchor);
        }

        m_dirty = true;
    }

	void LabelNode::OnTranformChanged()
    {
        bool batch_chaging = false;
        std::shared_ptr<Transform> t = GetTransform();
        auto batch = m_batch.lock();

        while(t && t->IsChangeNotifying())
        {
            auto batch_parent = t->GetGameObject()->GetComponent<LabelBatchRenderer>();
            if(batch_parent && batch_parent == batch)
            {
                batch_chaging = true;
                break;
            }

            t = t->GetParent().lock();
        }

        if(!batch_chaging)
        {
            m_dirty = true;
        }
    }

	void LabelNode::SetLabel(const std::shared_ptr<Label> &label)
    {
        if(m_label != label)
        {
            m_label = label;
            m_dirty = true;
        }
    }

	void LabelNode::SetColor(const Color &color)
    {
        if(m_color != color)
        {
            m_color = color;
            m_dirty = true;
        }
    }

    void LabelNode::SetSortingOrder(int order)
    {
        if(m_sorting_order != order)
        {
            m_sorting_order = order;
            m_dirty = true;
        }
    }

	void LabelNode::SetAnchor(const Vector4 &anchor)
    {
        if(!m_anchor || *m_anchor != anchor)
        {
            m_anchor = std::make_shared<Vector4>(anchor);
            m_dirty = true;
        }
    }
}
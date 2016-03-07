#include "SpriteNode.h"
#include "GameObject.h"
#include "UICanvas.h"
#include "SpriteBatchRenderer.h"

namespace Galaxy3D
{
	DEFINE_COM_CLASS(SpriteNode);

	void SpriteNode::DeepCopy(std::shared_ptr<Object> &source)
	{
		auto src_node = std::dynamic_pointer_cast<SpriteNode>(source);

		Component::DeepCopy(source);

		m_sprite = src_node->m_sprite;
		m_color = src_node->m_color;
		m_anchor = src_node->m_anchor;
		m_sorting_order = src_node->m_sorting_order;
		m_dirty = true;
	}

    void SpriteNode::Start()
    {
        auto canvas = GetGameObject()->GetComponentInParent<UICanvas>();

        if(canvas && m_anchor)
        {
            canvas->AnchorTransform(GetTransform(), *m_anchor);
        }

        m_dirty = true;
    }

    void SpriteNode::OnTranformChanged()
    {
        bool batch_chaging = false;
        std::shared_ptr<Transform> t = GetTransform();
        auto batch = m_batch.lock();

        while(t && t->IsChangeNotifying())
        {
            auto batch_parent = t->GetGameObject()->GetComponent<SpriteBatchRenderer>();
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

    void SpriteNode::SetSprite(const std::shared_ptr<Sprite> &sprite)
    {
        if(m_sprite != sprite)
        {
            m_sprite = sprite;
            m_dirty = true;
        }
    }

    void SpriteNode::SetColor(const Color &color)
    {
        if(m_color != color)
        {
            m_color = color;
            m_dirty = true;
        }
    }

    void SpriteNode::SetSortingOrder(int order)
    {
        if(m_sorting_order != order)
        {
            m_sorting_order = order;
            m_dirty = true;
        }
    }

    void SpriteNode::SetAnchor(const Vector4 &anchor)
    {
        if(!m_anchor || *m_anchor != anchor)
        {
            m_anchor = std::make_shared<Vector4>(anchor);
            m_dirty = true;
        }
    }
}
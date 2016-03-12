#include "Octree.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Debug.h"

namespace Galaxy3D
{
    void OctreeNode::SetVisible(bool visible)
    {
        for(size_t i=0; i<renderers.size(); i++)
        {
            renderers[i].lock()->SetVisible(visible);
        }

        for(int i=0; i<8; i++)
        {
            auto &child = children[i];

            if(child)
            {
                child->SetVisible(visible);
            }
        }
    }

    Octree::Octree(const std::shared_ptr<GameObject> &obj)
    {
        auto renderers = obj->GetComponentsInChildren<Renderer>();

        Vector3 box_max = Vector3(1, 1, 1) * Mathf::MinFloatValue;
        Vector3 box_min = Vector3(1, 1, 1) * Mathf::MaxFloatValue;
        
        m_root = std::shared_ptr<OctreeNode>(new OctreeNode());
        
        for(size_t i=0; i<renderers.size(); i++)
        {
            auto bounds = renderers[i]->GetBounds();
            
            box_max = Vector3::Max(box_max, bounds.GetMax());
            box_min = Vector3::Min(box_min, bounds.GetMin());

            m_root->renderers.push_back(renderers[i]);
        }

        m_root->center = (box_max + box_min) * 0.5f;
        m_root->extents = (box_max - box_min) * 0.5f;
        m_root->depth = 0;

        SplitNode(m_root);
    }

    void Octree::SplitNode(const std::shared_ptr<OctreeNode> &node)
    {
        //Debug::Log("%d %d", node->renderers.size(), node->depth);

        if(node->renderers.size() <= 1)
        {
            return;
        }

        Vector3 center_signs[8] = {
            Vector3(1, 1, 1),
            Vector3(-1, 1, 1),
            Vector3(-1, 1, -1),
            Vector3(1, 1, -1),
            Vector3(1, -1, 1),
            Vector3(-1, -1, 1),
            Vector3(-1, -1, -1),
            Vector3(1, -1, -1)
        };

        std::vector<std::weak_ptr<Renderer>> renderers = node->renderers;

        for(int i=0; i<8; i++)
        {
            auto child = std::shared_ptr<OctreeNode>(new OctreeNode());
            child->center = Vector3(
                node->center.x + node->extents.x * 0.5f * center_signs[i].x,
                node->center.y + node->extents.y * 0.5f * center_signs[i].y,
                node->center.z + node->extents.z * 0.5f * center_signs[i].z);
            child->extents = node->extents * 0.5f;
            child->depth = node->depth + 1;
            child->parent = node;

            node->children[i] = child;

            for(size_t j=0; j<renderers.size(); )
            {
                auto &renderer = renderers[j];
                const Bounds &bounds = renderer.lock()->GetBounds();
                
                if(IsNodeContainBounds(node->children[i], bounds))
                {
                    child->renderers.push_back(renderer);
                    renderers.erase(renderers.begin() + j);
                    continue;
                }

                j++;
            }
        }

        node->renderers = renderers;

        // split children
        for(int i=0; i<8; i++)
        {
            SplitNode(node->children[i]);
        }
    }

    bool Octree::IsNodeContainBounds(const std::shared_ptr<OctreeNode> &node, const Bounds &bounds)
    {
        Vector3 node_max = node->center + node->extents * 0.5f;
        Vector3 node_min = node->center - node->extents * 0.5f;
        Vector3 bounds_max = bounds.GetMax();
        Vector3 bounds_min = bounds.GetMin();

        if( node_min.x <= bounds_min.x &&
            node_min.y <= bounds_min.y &&
            node_min.z <= bounds_min.z &&
            node_max.x >= bounds_max.x &&
            node_max.y >= bounds_max.y &&
            node_max.z >= bounds_max.z)
        {
            return true;
        }

        return false;
    }
}
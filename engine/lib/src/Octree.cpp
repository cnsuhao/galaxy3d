#include "Octree.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Debug.h"

namespace Galaxy3D
{
    Octree::Octree(const std::shared_ptr<GameObject> &obj)
    {
        auto renderers = obj->GetComponentsInChildren<Renderer>();

        Vector3 box_max = Vector3(1, 1, 1) * Mathf::MinFloatValue;
        Vector3 box_min = Vector3(1, 1, 1) * Mathf::MaxFloatValue;
        
        for(size_t i=0; i<renderers.size(); i++)
        {
            auto &bounds = renderers[i]->GetBounds();
            
            box_max = Vector3::Max(box_max, bounds.GetMax());
            box_min = Vector3::Min(box_min, bounds.GetMin());
        }

        m_root = std::shared_ptr<OctreeNode>(new OctreeNode());
        m_root->center = (box_max + box_min) * 0.5f;
        m_root->extents = (box_max - box_min) * 0.5f;
        
        SplitNode(m_root, renderers);

        Debug::Log("%d %s %s", renderers.size(), m_root->center.ToString().c_str(), m_root->extents.ToString().c_str());
    }

    void Octree::SplitNode(const std::shared_ptr<OctreeNode> &node, const std::vector<std::shared_ptr<Renderer>> &renderers)
    {

    }
}
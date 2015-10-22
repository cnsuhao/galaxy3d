#ifndef __Octree_h__
#define __Octree_h__

#include "Vector3.h"
#include <vector>
#include <memory>

namespace Galaxy3D
{
    class GameObject;
    class Renderer;

    struct OctreeNode
    {
        Vector3 center;
        Vector3 extents;
        //std::weak_ptr<Renderer> renderer;
        std::shared_ptr<OctreeNode> parent;
        std::shared_ptr<OctreeNode> children[8];
    };

    class Octree
    {
    public:
        Octree(const std::shared_ptr<GameObject> &obj);

    private:
        void SplitNode(const std::shared_ptr<OctreeNode> &node, const std::vector<std::shared_ptr<Renderer>> &renderers);

    private:
        std::shared_ptr<OctreeNode> m_root;
    };
}

#endif
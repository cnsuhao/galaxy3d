#ifndef __Octree_h__
#define __Octree_h__

#include "Vector3.h"
#include "Bounds.h"
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
        int depth;
        std::weak_ptr<OctreeNode> parent;
        std::shared_ptr<OctreeNode> children[8];
        std::vector<std::weak_ptr<Renderer>> renderers;

        void SetVisible(bool visible);
    };

    class Octree
    {
    public:
        Octree(const std::shared_ptr<GameObject> &obj);
        std::shared_ptr<OctreeNode> GetRootNode() const {return m_root;}

    private:
        std::shared_ptr<OctreeNode> m_root;

        static void SplitNode(const std::shared_ptr<OctreeNode> &node);
        static bool IsNodeContainBounds(const std::shared_ptr<OctreeNode> &node, const Bounds &bounds);
    };
}

#endif
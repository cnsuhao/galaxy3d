#include "SpriteNode.h"
#include "GameObject.h"
#include "UICanvas.h"

namespace Galaxy3D
{
    void SpriteNode::AnchorTransform()
    {
        auto canvas = GetGameObject()->GetComponentInParent<UICanvas>();

        if(canvas && m_anchor)
        {
            canvas->AnchorTransform(GetTransform(), *m_anchor);
        }
    }

    void SpriteNode::SetAnchor(const Vector4 &anchor)
    {
        m_anchor = std::make_shared<Vector4>(anchor);
    }
}
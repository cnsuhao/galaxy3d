#ifndef __HighlightingObject_h__
#define __HighlightingObject_h__

#include "Component.h"
#include "Color.h"
#include "Renderer.h"

namespace Galaxy3D
{
    class HighlightingObject : public Component
    {
    public:
        void SetColor(const Color &color) {m_color = color;}

    protected:
        virtual void Start();
        virtual void OnWillRenderObject(int material_index);

    private:
        Color m_color;
        std::vector<std::shared_ptr<Material>> m_materials;
        std::shared_ptr<Shader> m_shader_highlighting;
        std::vector<std::shared_ptr<Shader>> m_shaders_default;
    };
}

#endif
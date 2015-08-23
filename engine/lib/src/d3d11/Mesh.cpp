#include "Mesh.h"
#include "Material.h"
#include "GTFile.h"
#include "MeshRenderer.h"
#include "ShaderPropertyType.h"
#include "Texture2D.h"

#define BUFFER_READ(dest, p, size) if(size > 0){memcpy(&dest, p, size);p += size;}

namespace Galaxy3D
{
    static std::string read_string(char *&p)
    {
        int size;

        BUFFER_READ(size, p, 4);
        std::string str(size, 0);
        BUFFER_READ(str[0], p, size);

        return str;
    }

    static std::shared_ptr<Material> read_material(char *&p, const std::string &dir)
    {
        std::string mat_name = read_string(p);
        std::string mat_guid = read_string(p);
        std::string shader_name = read_string(p);

        auto mat = Material::Create(shader_name);

        int property_count;
        BUFFER_READ(property_count, p, 4);

        for(int i=0; i<property_count; i++)
        {
            int property_type;
            BUFFER_READ(property_type, p, 4);
            auto type = (ShaderPropertyType::Enum) property_type;

            std::string property_name = read_string(p);

            switch(type)
            {
                case ShaderPropertyType::Color:
                {
                    Color value;
                    BUFFER_READ(value, p, sizeof(Color));

                    mat->SetColor(property_name, value);
                }
                break;
                case ShaderPropertyType::Float:
                case ShaderPropertyType::Range:
                {
                    float value;
                    BUFFER_READ(value, p, 4);

                    mat->SetVector(property_name, Vector4(value, 0, 0, 0));
                }
                break;
                case ShaderPropertyType::TexEnv:
                {
                    std::string tex_name = read_string(p);
                    std::string tex_path = dir + "/" + tex_name;

                    auto tex = Texture2D::LoadFromFile(tex_path, FilterMode::Bilinear, TextureWrapMode::Clamp);

                    mat->SetTexture(property_name, tex);
                }
                break;
                case ShaderPropertyType::Vector:
                {
                    Vector4 value;
                    BUFFER_READ(value, p, sizeof(Vector4));

                    mat->SetVector(property_name, value);
                }
                break;
            }
        }

        return mat;
    }

    Mesh::Mesh():
        m_vertex_buffer(nullptr),
        m_index_buffer(nullptr)
    {
    }

    std::shared_ptr<Mesh> Mesh::Create()
    {
        std::shared_ptr<Mesh> mesh(new Mesh());
        return mesh;
    }

    std::shared_ptr<GameObject> Mesh::LoadFromFile(const std::string &file)
    {
        std::shared_ptr<GameObject> obj;

        if(GTFile::Exist(file))
        {
            auto bytes = GTFile::ReadAllBytes(file);
            char *p = &bytes[0];

            std::string obj_name = read_string(p);
            obj = GameObject::Create(obj_name);

            int renderer_count;
            BUFFER_READ(renderer_count, p, 4);
            for(int i=0; i<renderer_count; i++)
            {
                std::string renderer_name = read_string(p);

                Vector3 pos;
                BUFFER_READ(pos, p, sizeof(Vector3));
                Quaternion rot;
                BUFFER_READ(rot, p, sizeof(Quaternion));
                Vector3 sca;
                BUFFER_READ(sca, p, sizeof(Vector3));

                auto renderer_obj = GameObject::Create(renderer_name);
                auto renderer_tran = renderer_obj->GetTransform();
                renderer_tran->SetParent(obj->GetTransform());
                renderer_tran->SetLocalPosition(pos);
                renderer_tran->SetLocalRotation(rot);
                renderer_tran->SetLocalScale(sca);

                auto renderer = renderer_obj->AddComponent<MeshRenderer>();

                ReadMesh(p, renderer.get(), file.substr(0, file.find_last_of('/')));

                int lightmap_index;
                BUFFER_READ(lightmap_index, p, 4);
                Vector4 lightmap_tiling_offset;
                BUFFER_READ(lightmap_tiling_offset, p, sizeof(Vector4));

                if(lightmap_index >= 0)
                {
                    renderer->SetLightmapIndex(lightmap_index);
                    renderer->SetLightmapTilingOffset(lightmap_tiling_offset);
                }
            }
        }

        return obj;
    }

    void Mesh::ReadMesh(char *&p, Renderer *r, const std::string &dir)
    {
        auto renderer = (MeshRenderer *) r;

        auto mesh = Galaxy3D::Mesh::Create();

        int vertex_count;
        BUFFER_READ(vertex_count, p, 4);

        mesh->m_vertices.resize(vertex_count);
        BUFFER_READ(mesh->m_vertices[0], p, vertex_count * sizeof(VertexMesh));

        int sub_count;
        BUFFER_READ(sub_count, p, 4);

        mesh->m_sub_indices.resize(sub_count);
        for(int i=0; i<sub_count; i++)
        {
            int index_count;
            BUFFER_READ(index_count, p, 4);

            mesh->m_sub_indices[i].resize(index_count);
            BUFFER_READ(mesh->m_sub_indices[i][0], p, index_count * 2);
        }

        renderer->SetMesh(mesh);

        int mat_count;
        BUFFER_READ(mat_count, p, 4);
        std::vector<std::shared_ptr<Material>> mats(mat_count);
        for(int i=0; i<mat_count; i++)
        {
            mats[i] = read_material(p, dir);
        }

        renderer->SetSharedMaterials(mats);
    }

    ID3D11Buffer *Mesh::GetVertexBuffer()
    {
        if(m_vertex_buffer == nullptr)
        {
            int buffer_size = sizeof(VertexMesh) * m_vertices.size();
            char *buffer = (char *) &m_vertices[0];

            bool dynamic = false;

            auto device = GraphicsDevice::GetInstance()->GetDevice();

            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
            bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.ByteWidth = buffer_size;

            D3D11_SUBRESOURCE_DATA data;
            ZeroMemory(&data, sizeof(data));
            data.pSysMem = buffer;
            HRESULT hr = device->CreateBuffer(&bd, &data, &m_vertex_buffer);
        }

        return m_vertex_buffer;
    }

    ID3D11Buffer *Mesh::GetIndexBuffer()
    {
        if(m_index_buffer == nullptr)
        {
            std::vector<unsigned short> uv;
            for(size_t i=0; i<m_sub_indices.size(); i++)
            {
                int size = m_sub_indices[i].size();
                int old_size = uv.size();
                uv.resize(old_size + size);
                memcpy(&uv[old_size], &m_sub_indices[i][0], size * 2);
            }

            int buffer_size = sizeof(unsigned short) * uv.size();
            char *buffer = (char *) &uv[0];

            auto device = GraphicsDevice::GetInstance()->GetDevice();

            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.CPUAccessFlags = 0;
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.ByteWidth = buffer_size;

            D3D11_SUBRESOURCE_DATA data;
            ZeroMemory(&data, sizeof(data));
            data.pSysMem = buffer;
            HRESULT hr = device->CreateBuffer(&bd, &data, &m_index_buffer);
        }

        return m_index_buffer;
    }
}
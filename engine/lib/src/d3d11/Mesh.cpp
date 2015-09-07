#include "Mesh.h"
#include "Texture2D.h"
#include "ShaderPropertyType.h"
#include "Material.h"
#include "GTFile.h"
#include "MeshRenderer.h"
#include "SkinnedMeshRenderer.h"
#include "Animation.h"
#include "LightmapSettings.h"
#include <unordered_map>

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

    static std::shared_ptr<Material> read_material(char *&p, const std::string &dir, bool skin)
    {
        std::string mat_name = read_string(p);
        std::string mat_guid = read_string(p);
        std::string shader_name = read_string(p);

        if(skin)
        {
            shader_name = "SkinnedMesh/" + shader_name;
        }

        if("Specular" == shader_name)
        {
            shader_name = "Diffuse";
        }
        else if("Nature/Tree Soft Occlusion Leaves" == shader_name)
        {
            shader_name = "Transparent/Cutout/Diffuse";
        }

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

                    auto tex = Texture2D::LoadFromFile(tex_path, FilterMode::Bilinear, TextureWrapMode::Clamp, true);

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

    Mesh::~Mesh()
    {
        SAFE_RELEASE(m_vertex_buffer);
        SAFE_RELEASE(m_index_buffer);
    }

    std::shared_ptr<Mesh> Mesh::Create()
    {
        std::shared_ptr<Mesh> mesh(new Mesh());
        return mesh;
    }

    std::shared_ptr<GameObject> Mesh::LoadSkinnedMesh(const std::string &file)
    {
        std::shared_ptr<GameObject> obj;

        if(GTFile::Exist(file))
        {
            auto bytes = GTFile::ReadAllBytes(file);
            char *p = &bytes[0];

            std::string obj_name = read_string(p);
            obj = GameObject::Create(obj_name);
            auto tran = obj->GetTransform();
            auto anim = obj->AddComponent<Animation>();

            std::unordered_map<std::string, std::shared_ptr<Transform>> bones;

            int bone_count;
            BUFFER_READ(bone_count, p, 4);
            for(int i=0; i<bone_count; i++)
            {
                std::string bone_path = read_string(p);
                std::string bone_name = read_string(p);

                Vector3 pos;
                BUFFER_READ(pos, p, sizeof(Vector3));
                Quaternion rot;
                BUFFER_READ(rot, p, sizeof(Quaternion));
                Vector3 sca;
                BUFFER_READ(sca, p, sizeof(Vector3));

                std::string parent = read_string(p);

                auto bone_tran = GameObject::Create(bone_name)->GetTransform();
                bone_tran->SetPosition(pos);
                bone_tran->SetRotation(rot);
                bone_tran->SetScale(sca);

                bones[bone_path] = bone_tran;

                if(parent == "")
                {
                    bone_tran->SetParent(tran);
                }
                else
                {
                    auto find = bones.find(parent);
                    if(find != bones.end())
                    {
                        bone_tran->SetParent(find->second);
                    }
                    else
                    {
                        bones[bone_path] = bone_tran;
                    }
                }
            }

            std::unordered_map<std::string, AnimationState> states;

            int clip_count;
            BUFFER_READ(clip_count, p, 4);
            for(int i=0; i<clip_count; i++)
            {
                std::string clip_name = read_string(p);

                float frame_rate;
                BUFFER_READ(frame_rate, p, 4);
                float length;
                BUFFER_READ(length, p, 4);
                char wrap_mode;
                BUFFER_READ(wrap_mode, p, 1);

                AnimationClip clip;
                clip.SetName(clip_name);
                clip.frame_rate = frame_rate;
                clip.length = length;
                clip.wrap_mode = (WrapMode::Enum) wrap_mode;

                int path_count;
                BUFFER_READ(path_count, p, 4);
                for(int j=0; j<path_count; j++)
                {
                    std::string path = read_string(p);

                    CurveBinding cb;
                    cb.path = path;

                    auto find = bones.find(path);
                    if(find != bones.end())
                    {
                        cb.transform = find->second;
                    }

                    int curve_count;
                    BUFFER_READ(curve_count, p, 4);

                    cb.curves.resize(curve_count);

                    for(int k=0; k<curve_count; k++)
                    {
                        int key_count;
                        BUFFER_READ(key_count, p, 4);

                        if(key_count > 0)
                        {
                            AnimationCurve &curve = cb.curves[k];
                            curve.keys.resize(key_count);
                            BUFFER_READ(curve.keys[0], p, sizeof(Keyframe) * key_count);
                        }
                    }

                    clip.curves[path] = cb;
                }

                AnimationState state(clip);
                states[clip_name] = state;
            }

            anim->SetAnimationStates(states);

            int skinn_count;
            BUFFER_READ(skinn_count, p, 4);
            for(int i=0; i<skinn_count; i++)
            {
                std::string renderer_name = read_string(p);

                Vector3 pos;
                BUFFER_READ(pos, p, sizeof(Vector3));
                Quaternion rot;
                BUFFER_READ(rot, p, sizeof(Quaternion));
                Vector3 sca;
                BUFFER_READ(sca, p, sizeof(Vector3));

                int renderer_bone_count;
                BUFFER_READ(renderer_bone_count, p, 4);
                std::vector<std::shared_ptr<Transform>> renderer_bones(renderer_bone_count);
                for(int j=0; j<renderer_bone_count; j++)
                {
                    std::string bone_name = read_string(p);

                    auto find = bones.find(bone_name);
                    if(find != bones.end())
                    {
                        renderer_bones[j] = find->second;
                    }
                }

                auto renderer_obj = GameObject::Create(renderer_name);
                auto renderer_tran = renderer_obj->GetTransform();
                renderer_tran->SetParent(obj->GetTransform());
                renderer_tran->SetPosition(pos);
                renderer_tran->SetRotation(rot);
                renderer_tran->SetScale(sca);

                auto renderer = renderer_obj->AddComponent<SkinnedMeshRenderer>();
                auto mesh = ReadMesh(p, renderer.get(), file.substr(0, file.find_last_of('/')), true);
                renderer->SetMesh(mesh);
                renderer->SetBones(renderer_bones);
            }

            int static_count;
            BUFFER_READ(static_count, p, 4);
            for(int i=0; i<static_count; i++)
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
                renderer_tran->SetPosition(pos);
                renderer_tran->SetRotation(rot);
                renderer_tran->SetScale(sca);

                auto renderer = renderer_obj->AddComponent<MeshRenderer>();
                auto mesh = ReadMesh(p, renderer.get(), file.substr(0, file.find_last_of('/')), false);
                renderer->SetMesh(mesh);

                std::string parent = read_string(p);

                auto find = bones.find(parent);
                if(find != bones.end())
                {
                    renderer_tran->SetParent(find->second);
                }
            }
        }

        return obj;
    }

    std::shared_ptr<GameObject> Mesh::LoadStaticMesh(const std::string &file)
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
                renderer_tran->SetPosition(pos);
                renderer_tran->SetRotation(rot);
                renderer_tran->SetScale(sca);

                auto renderer = renderer_obj->AddComponent<MeshRenderer>();

                auto mesh = ReadMesh(p, renderer.get(), file.substr(0, file.find_last_of('/')), false);

                renderer->SetMesh(mesh);

                int lightmap_index;
                BUFFER_READ(lightmap_index, p, 4);
                Vector4 lightmap_tiling_offset;
                BUFFER_READ(lightmap_tiling_offset, p, sizeof(Vector4));

                if(lightmap_index >= 0)
                {
                    renderer->SetLightmapIndex(lightmap_index);
                    renderer->SetLightmapTilingOffset(lightmap_tiling_offset);

                    if(lightmap_index < (int) LightmapSettings::lightmaps.size())
                    {
                        auto mat = renderer->GetSharedMaterial();
                        mat->SetVector("_LightmapST", lightmap_tiling_offset);
                        mat->SetTexture("_Lightmap", LightmapSettings::lightmaps[lightmap_index]);
                    }
                }
            }
        }

        return obj;
    }

    std::shared_ptr<Mesh> Mesh::ReadMesh(char *&p, Renderer *renderer, const std::string &dir, bool skin)
    {
        auto mesh = Mesh::Create();

        int vertex_count;
        BUFFER_READ(vertex_count, p, 4);

        if(skin)
        {
            mesh->m_vertices_skinned.resize(vertex_count);
            BUFFER_READ(mesh->m_vertices_skinned[0], p, vertex_count * sizeof(VertexSkinned));
        }
        else
        {
            mesh->m_vertices.resize(vertex_count);
            BUFFER_READ(mesh->m_vertices[0], p, vertex_count * sizeof(VertexMesh));
        }

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

        if(skin)
        {
            int pose_count;
            BUFFER_READ(pose_count, p, 4);
            mesh->m_bind_poses.resize(pose_count);
            BUFFER_READ(mesh->m_bind_poses[0], p, pose_count * sizeof(Matrix4x4));
        }

        int mat_count;
        BUFFER_READ(mat_count, p, 4);
        std::vector<std::shared_ptr<Material>> mats(mat_count);
        for(int i=0; i<mat_count; i++)
        {
            mats[i] = read_material(p, dir, skin);
        }

        renderer->SetSharedMaterials(mats);

        return mesh;
    }

    ID3D11Buffer *Mesh::GetVertexBuffer()
    {
        if(m_vertex_buffer == nullptr)
        {
            int buffer_size;
            char *buffer;

            if(!m_vertices.empty())
            {
                buffer_size = sizeof(VertexMesh) * m_vertices.size();
                buffer = (char *) &m_vertices[0];
            }
            else if(!m_vertices_skinned.empty())
            {
                buffer_size = sizeof(VertexSkinned) * m_vertices_skinned.size();
                buffer = (char *) &m_vertices_skinned[0];
            }
            else
            {
                return nullptr;
            }

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
        if(m_index_buffer == nullptr && !m_sub_indices.empty())
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
#include "Mesh.h"
#include "Texture2D.h"
#include "ShaderPropertyType.h"
#include "Material.h"
#include "GTFile.h"
#include "MeshRenderer.h"
#include "SkinnedMeshRenderer.h"
#include "Animation.h"
#include "LightmapSettings.h"
#include "GTString.h"
#include "Guid.h"
#include "Debug.h"
#include <unordered_map>

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

    std::shared_ptr<Material> Mesh::ReadMaterial(char *&p, const std::string &dir, bool skin)
    {
        std::string mat_guid = read_string(p);
        char has_data;
        BUFFER_READ(has_data, p, 1);

        if(!has_data)
        {
            return std::dynamic_pointer_cast<Material, Object>(FindCachedObject(mat_guid));
        }

        std::string mat_name = read_string(p);
        std::string shader_name = read_string(p);

        if(GTString(shader_name).StartsWith("Legacy Shaders/"))
        {
            shader_name = shader_name.substr(std::string("Legacy Shaders/").size());
        }

        if(GTString(shader_name).StartsWith("Particles/"))
        {
            shader_name = GTString(shader_name).Replace("Particles", "Effect").str;
        }

        if(skin)
        {
            shader_name = "SkinnedMesh/" + shader_name;
        }

        shader_name = GTString(shader_name).Replace(" ", "").str;

        auto mat = Material::Create(shader_name);
        mat->SetGuid(mat_guid);

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

                    if(mat)
                    {
                        mat->SetColor(property_name, value);
                    }
                }
                break;
                case ShaderPropertyType::Float:
                case ShaderPropertyType::Range:
                {
                    float value;
                    BUFFER_READ(value, p, 4);

                    if(mat)
                    {
                        mat->SetVector(property_name, Vector4(value, 0, 0, 0));
                    }
                }
                break;
                case ShaderPropertyType::TexEnv:
                {
                    std::string tex_name = read_string(p);
                    std::string tex_path = dir + "/" + tex_name;

                    if(!tex_name.empty())
                    {
                        TextureWrapMode::Enum wrap;
                        if(tex_name.back() == '0')
                        {
                            wrap = TextureWrapMode::Repeat;
                        }
                        else
                        {
                            wrap = TextureWrapMode::Clamp;
                        }

                        auto tex = Texture2D::LoadFromFile(tex_path, FilterMode::Bilinear, wrap, true);

                        if(mat)
                        {
                            mat->SetTexture(property_name, tex);
                        }
                    }
                }
                break;
                case ShaderPropertyType::Vector:
                {
                    Vector4 value;
                    BUFFER_READ(value, p, sizeof(Vector4));

                    if(mat)
                    {
                        mat->SetVector(property_name, value);
                    }
                }
                break;
            }
        }

        CacheObject(mat_guid, mat);

        return mat;
    }

    Mesh::Mesh()
    {
    }

    Mesh::~Mesh()
    {
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_vertex_buffer);
        GraphicsDevice::GetInstance()->ReleaseBufferObject(m_index_buffer);
    }

    std::shared_ptr<Mesh> Mesh::Create()
    {
        std::shared_ptr<Mesh> mesh(new Mesh());
        mesh->SetGuid(Guid::NewGuid());
        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::CreateMeshCube()
    {
        auto mesh = Mesh::Create();

        std::vector<VertexMesh> vertices;
        std::vector<unsigned short> indices;

        vertices.push_back({Vector3(-1, 1, -1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(-1, -1, -1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(1, -1, -1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(1, 1, -1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(-1, 1, 1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(-1, -1, 1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(1, -1, 1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(1, 1, 1) * 0.5f, Vector3(), Vector4(), Vector2(), Vector2()});

        unsigned short indices_[36] =
        {
            3, 6, 2, 3, 7, 6,//right
            4, 1, 5, 4, 0, 1,//left
            4, 3, 0, 4, 7, 3,//up
            1, 6, 5, 1, 2, 6,//down
            7, 5, 6, 7, 4, 5,//front
            0, 2, 1, 0, 3, 2,//back
        };
        indices.resize(36);
        memcpy(&indices[0], indices_, sizeof(indices_));

        mesh->SetVertices(vertices);
        mesh->SetIndices(std::vector<std::vector<unsigned short>>(1, indices));

        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::CreateMeshSphere()
    {
        auto mesh = Mesh::Create();

        std::vector<VertexMesh> vertices;
        std::vector<unsigned short> indices;

        vertices.push_back({Vector3(0, 1, 0), Vector3(), Vector4(), Vector2(), Vector2()});

        float rot_x;
        float rot_y;
        int x_count = 10;
        int y_count = 10;
        for(int i=0; i<x_count; i++)
        {
            rot_x = 180.0f / x_count * i;

            for(int j=0; j<y_count; j++)
            {
                if(i > 0)
                {
                    rot_y = 360.0f / y_count * j;

                    auto pos = Quaternion::Euler(rot_x, rot_y, 0) * Vector3(0, 1, 0);

                    VertexMesh v;
                    v.POSITION = pos;
                    vertices.push_back(v);
                }

                if(i == 0)
                {
                    if(j == y_count - 1)
                    {
                        indices.push_back(0);
                        indices.push_back(1 + j);
                        indices.push_back(1 + 0);
                    }
                    else
                    {
                        indices.push_back(0);
                        indices.push_back(1 + j);
                        indices.push_back(1 + j + 1);
                    }
                }
                else if(i == x_count - 1)
                {
                    if(j == y_count - 1)
                    {
                        indices.push_back(1 + y_count * (i - 1) + j);
                        indices.push_back(1 + y_count * i);
                        indices.push_back(1 + y_count * (i - 1) + 0);
                    }
                    else
                    {
                        indices.push_back(1 + y_count * (i - 1) + j);
                        indices.push_back(1 + y_count * i);
                        indices.push_back(1 + y_count * (i - 1) + j + 1);
                    }
                }
                else
                {
                    if(j == y_count - 1)
                    {
                        indices.push_back(1 + y_count * (i - 1) + j);
                        indices.push_back(1 + y_count * (i) + 0);
                        indices.push_back(1 + y_count * (i - 1) + 0);

                        indices.push_back(1 + y_count * (i - 1) + j);
                        indices.push_back(1 + y_count * (i) + j);
                        indices.push_back(1 + y_count * (i) + 0);
                    }
                    else
                    {
                        indices.push_back(1 + y_count * (i - 1) + j);
                        indices.push_back(1 + y_count * (i) + j + 1);
                        indices.push_back(1 + y_count * (i - 1) + j + 1);

                        indices.push_back(1 + y_count * (i - 1) + j);
                        indices.push_back(1 + y_count * (i) + j);
                        indices.push_back(1 + y_count * (i) + j + 1);
                    }
                }
            }
        }

        vertices.push_back({Vector3(0, -1, 0), Vector3(), Vector4(), Vector2(), Vector2()});

        mesh->SetVertices(vertices);
        mesh->SetIndices(std::vector<std::vector<unsigned short>>(1, indices));

        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::CreateMeshCone()
    {
        auto mesh = Mesh::Create();

        std::vector<VertexMesh> vertices;
        std::vector<unsigned short> indices;

        vertices.push_back({Vector3(0, 0, 0), Vector3(), Vector4(), Vector2(), Vector2()});
        vertices.push_back({Vector3(0, 0, 1), Vector3(), Vector4(), Vector2(), Vector2()});

        int z_count = 64;
        for(int i=0; i<z_count; i++)
        {
            float rot_z = 360.0f / z_count * i;
            auto pos = Quaternion::Euler(0, 0, rot_z) * Vector3(0, 1, 1);

            VertexMesh v;
            v.POSITION = pos;
            vertices.push_back(v);

            if(i == z_count - 1)
            {
                indices.push_back(0);
                indices.push_back(2 + 0);
                indices.push_back(2 + i);

                indices.push_back(1);
                indices.push_back(2 + i);
                indices.push_back(2 + 0);
            }
            else
            {
                indices.push_back(0);
                indices.push_back(2 + i + 1);
                indices.push_back(2 + i);

                indices.push_back(1);
                indices.push_back(2 + i);
                indices.push_back(2 + i + 1);
            }
        }

        mesh->SetVertices(vertices);
        mesh->SetIndices(std::vector<std::vector<unsigned short>>(1, indices));

        return mesh;
    }

    std::shared_ptr<GameObject> Mesh::LoadSkinnedMesh(const std::string &file)
    {
        std::shared_ptr<GameObject> obj;

        if(GTFile::Exist(file))
        {
            int file_size;
            void *bytes = GTFile::ReadAllBytes(file, &file_size);
            char *p = (char *) bytes;

            std::string obj_name = read_string(p);
            obj = GameObject::Create(obj_name);
            auto tran = obj->GetTransform();
            auto anim = obj->AddComponent<Animation>();

            std::unordered_map<std::string, std::shared_ptr<Transform>> bones;

            int bone_count;
            BUFFER_READ(bone_count, p, 4);
            std::vector<std::string> bone_paths;
            std::vector<std::string> bone_parents;

            // create bones
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

                bone_paths.push_back(bone_path);
                bone_parents.push_back(parent);
            }

            // set parent for every bone
            for(int i=0; i<bone_count; i++)
            {
                std::string bone_path = bone_paths[i];
                std::string parent = bone_parents[i];
                auto bone_tran = bones[bone_path];

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
                }
            }

            anim->SetBones(bones);

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
                
                auto clip = std::shared_ptr<AnimationClip>(new AnimationClip());
                clip->SetName(clip_name);
                clip->frame_rate = frame_rate;
                clip->length = length;
                clip->wrap_mode = (WrapMode::Enum) wrap_mode;

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

                    clip->curves[path] = cb;
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
                else
                {
                    renderer_tran->SetParent(tran);
                }
            }

            free(bytes);

            Renderer::SortAllBatches();
        }

        return obj;
    }

    std::shared_ptr<GameObject> Mesh::LoadStaticMesh(const std::string &file)
    {
        std::shared_ptr<GameObject> obj;

        if(GTFile::Exist(file))
        {
            int file_size;
            void *bytes = GTFile::ReadAllBytes(file, &file_size);
            char *p = (char *) bytes;

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
                if(mesh)
                {
                    renderer->SetMesh(mesh);
                    renderer->CalculateBounds();
                }
                
                // lightmap
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
                        auto &mats = renderer->GetSharedMaterials();

                        for(size_t i=0; i<mats.size(); i++)
                        {
                            auto &mat = mats[i];
                            if(mat && !GTString(mat->GetShader()->GetName()).StartsWith("Lightmap/"))
                            {
                                mat->SetShader(Shader::Find("Lightmap/" + mat->GetShader()->GetName()));
                            }
                        }
                    }
                }
            }

            free(bytes);

            Renderer::SortAllBatches();
        }

        return obj;
    }

    std::shared_ptr<Mesh> Mesh::ReadMesh(char *&p, Renderer *renderer, const std::string &dir, bool skin)
    {
        std::shared_ptr<Mesh> mesh;

        std::string mesh_guid = read_string(p);
        char has_data;
        BUFFER_READ(has_data, p, 1);

        if(!has_data)
        {
            mesh = std::dynamic_pointer_cast<Mesh, Object>(FindCachedObject(mesh_guid));
        }
        else
        {
            mesh = Mesh::Create();
            mesh->SetGuid(mesh_guid);

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

            CacheObject(mesh_guid, mesh);
        }

        int mat_count;
        BUFFER_READ(mat_count, p, 4);
        std::vector<std::shared_ptr<Material>> mats(mat_count);
        for(int i=0; i<mat_count; i++)
        {
            mats[i] = ReadMaterial(p, dir, skin);
        }

        renderer->SetSharedMaterials(mats, false);

        return mesh;
    }

    void Mesh::CalculateBounds()
    {
        Vector3 box_max = Vector3(1, 1, 1) * Mathf::MinFloatValue;
        Vector3 box_min = Vector3(1, 1, 1) * Mathf::MaxFloatValue;
        for(size_t j=0; j<m_vertices.size(); j++)
        {
            auto &v = m_vertices[j].POSITION;

            box_max = Vector3::Max(box_max, v);
            box_min = Vector3::Min(box_min, v);
        }
        m_bounds = Bounds((box_max + box_min) * 0.5f, (box_max - box_min) * 0.5f);
    }

    BufferObject Mesh::GetVertexBuffer()
    {
        if(m_vertex_buffer.buffer == NULL)
        {
            int buffer_size = 0;
            char *buffer = NULL;

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

            m_vertex_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Vertex);
        }

        return m_vertex_buffer;
    }

    BufferObject Mesh::GetIndexBuffer()
    {
        if(m_index_buffer.buffer == NULL && !m_sub_indices.empty())
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

            m_index_buffer = GraphicsDevice::GetInstance()->CreateBufferObject(buffer, buffer_size, BufferUsage::StaticDraw, BufferType::Index);
        }
        
        return m_index_buffer;
    }
}
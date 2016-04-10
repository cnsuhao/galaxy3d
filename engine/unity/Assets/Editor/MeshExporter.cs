using UnityEngine;
using System.Collections.Generic;
using UnityEditor;
using System.IO;

public class MeshExporter
{
    [MenuItem("Geart3D/Export All Meshes")]
    public static void ExportAllMeshes()
    {
        if(Selection.gameObjects == null || Selection.gameObjects.Length != 1)
        {
            return;
        }

        GameObject obj = Selection.activeGameObject;
        Dictionary<string, Material> shared_mats = new Dictionary<string, Material>();
        Dictionary<string, Mesh> shared_meshes = new Dictionary<string, Mesh>();
        
        string path = Application.dataPath + "/" + obj.name + ".mesh";
        BinaryWriter bw = new BinaryWriter(File.Open(path, FileMode.Create));

        WriteString(bw, obj.name);

        var rs = obj.GetComponentsInChildren<MeshRenderer>();
        bw.Write(rs.Length);
        for(int i = 0; i < rs.Length; i++)
        {
            var r = rs[i];

            WriteString(bw, r.gameObject.name);

            WriteVector3(bw, r.transform.position);
            WriteQuaternion(bw, r.transform.rotation);
            WriteVector3(bw, r.transform.lossyScale);

            var mesh = r.GetComponent<MeshFilter>().sharedMesh;

            WriteMesh(bw, mesh, r.sharedMaterials, false, shared_mats, shared_meshes);
            WriteLightmap(bw, r);
        }

        bw.Close();

        Debug.Log("export mesh " + obj.name + " done.");
    }

    enum CurveProperty
    {
        LocalPosX,
        LocalPosY,
        LocalPosZ,
        LocalRotX,
        LocalRotY,
        LocalRotZ,
        LocalRotW,
        LocalScaX,
        LocalScaY,
        LocalScaZ,

        Count
    }

    class BoneClip
    {
        public string path;
        public AnimationCurve[] curves;
    }

    [MenuItem("Geart3D/Export One Animation")]
    public static void ExportOneAnimation()
    {
        if(Selection.gameObjects == null || Selection.gameObjects.Length != 1)
        {
            return;
        }

        GameObject obj = Selection.activeGameObject;
        var anim = obj.GetComponent<Animation>();
        if(anim == null)
        {
            return;
        }

        Dictionary<string, Material> shared_mats = new Dictionary<string, Material>();
        Dictionary<string, Mesh> shared_meshes = new Dictionary<string, Mesh>();
        Dictionary<string, Dictionary<string, BoneClip>> clip_bone_clips = new Dictionary<string, Dictionary<string, BoneClip>>();
        Dictionary<string, Transform> bone_names = new Dictionary<string, Transform>();

        AnimationClip[] clips = AnimationUtility.GetAnimationClips(anim.gameObject);
        for(int i=0; i<clips.Length; i++)
        {
            var clip = clips[i];
            EditorCurveBinding[] cbs = AnimationUtility.GetCurveBindings(clip);

            Dictionary<string, BoneClip> bone_clips = new Dictionary<string, BoneClip>();
            clip_bone_clips.Add(clip.name, bone_clips);

            for(int j=0; j< cbs.Length; j++)
            {
                var cb = cbs[j];

                BoneClip bc;

                if(!bone_names.ContainsKey(cb.path))
                {
                    Transform t = obj.transform.Find(cb.path);
                    if(t)
                    {
                        bone_names.Add(cb.path, t);
                    }
                }

                if(!bone_clips.ContainsKey(cb.path))
                {
                    bc = new BoneClip();
                    bc.path = cb.path;
                    bc.curves = new AnimationCurve[(int) CurveProperty.Count];

                    bone_clips.Add(bc.path, bc);
                }
                else
                {
                    bc = bone_clips[cb.path];
                }

                int property = -1;

                switch(cb.propertyName)
                {
                case "m_LocalPosition.x":
                    property = 0;
                    break;
                case "m_LocalPosition.y":
                    property = 1;
                    break;
                case "m_LocalPosition.z":
                    property = 2;
                    break;
                case "m_LocalRotation.x":
                    property = 3;
                    break;
                case "m_LocalRotation.y":
                    property = 4;
                    break;
                case "m_LocalRotation.z":
                    property = 5;
                    break;
                case "m_LocalRotation.w":
                    property = 6;
                    break;
                case "m_LocalScale.x":
                    property = 7;
                    break;
                case "m_LocalScale.y":
                    property = 8;
                    break;
                case "m_LocalScale.z":
                    property = 9;
                    break;
                }

                if(property >= 0 && property < bc.curves.Length)
                {
                    bc.curves[property] = AnimationUtility.GetEditorCurve(clip, cb);
                }
            }
        }

        //add bones without clip curve
        List<string> add_bones = new List<string>();
        foreach(var i in bone_names)
        {
            Transform t = i.Value.parent;
            if(t == null)
            {
                continue;
            }

            string transform_path = t.name;

            if(t == obj.transform)
            {
                transform_path = "";
            }

            while(t != obj.transform)
            {
                if(t.parent != obj.transform)
                {
                    transform_path = t.parent.name + "/" + transform_path;
                }

                t = t.parent;
            }

            if(transform_path != "" && !bone_names.ContainsKey(transform_path) && !add_bones.Contains(transform_path))
            {
                add_bones.Add(transform_path);
            }
        }

        for(int i=0; i<add_bones.Count; i++)
        {
            bone_names.Add(add_bones[i], obj.transform.Find(add_bones[i]));
        }

        string path = Application.dataPath + "/" + obj.name + ".anim";
        BinaryWriter bw = new BinaryWriter(File.Open(path, FileMode.Create));

        WriteString(bw, obj.name);
        
        //write bones
        bw.Write(bone_names.Count);
        foreach(var i in bone_names)
        {
            Transform t = i.Value;

            WriteString(bw, i.Key);

            WriteString(bw, t.name);
            WriteVector3(bw, t.position);
            WriteQuaternion(bw, t.rotation);
            WriteVector3(bw, t.lossyScale);

            string parent = "";
            if(t.parent != anim.transform)
            {
                foreach(var j in bone_names)
                {
                    if(j.Value == t.parent)
                    {
                        parent = j.Key;
                        break;
                    }
                }
            }
            WriteString(bw, parent);
        }

        Debug.Log(bw.BaseStream.Position);

        //write anim clips
        bw.Write(clip_bone_clips.Count);
        for(int i=0; i<clips.Length; i++)
        {
            var clip = clips[i];
            
            WriteString(bw, clip.name);
            bw.Write(clip.frameRate);
            bw.Write(clip.length);
            bw.Write((byte) clip.wrapMode);

            var cbc = clip_bone_clips[clip.name];
            bw.Write(cbc.Count);
            foreach(var j in cbc)
            {
                BoneClip bc = j.Value;

                WriteString(bw, j.Key);

                bw.Write(bc.curves.Length);
                for(int k=0; k<bc.curves.Length; k++)
                {
                    var c = bc.curves[k];

                    if(c == null)
                    {
                        bw.Write(0);
                    }
                    else
                    {
                        var keys = c.keys;
                        bw.Write(keys.Length);

                        for(int m=0; m<keys.Length; m++)
                        {
                            Keyframe f = keys[m];
                        
                            bw.Write(f.inTangent);
                            bw.Write(f.outTangent);
                            bw.Write(f.tangentMode);
                            bw.Write(f.time);
                            bw.Write(f.value);
                        }
                    }
                }
            }
        }

        Debug.Log(bw.BaseStream.Position);
        
        SkinnedMeshRenderer[] renderers = anim.GetComponentsInChildren<SkinnedMeshRenderer>();
        bw.Write(renderers.Length);
        foreach(var r in renderers)
        {
            WriteString(bw, r.gameObject.name);

            WriteVector3(bw, r.transform.position);
            WriteQuaternion(bw, r.transform.rotation);
            WriteVector3(bw, r.transform.lossyScale);

            //write renderer bones index
            bw.Write(r.bones.Length);
            for(int i=0; i<r.bones.Length; i++)
            {
                string bone = "";
                foreach(var j in bone_names)
                {
                    if(j.Value == r.bones[i])
                    {
                        bone = j.Key;
                        break;
                    }
                }

                WriteString(bw, bone);
            }

            Mesh mesh = r.sharedMesh;
            if(mesh == null)
            {
                mesh = new Mesh();
                Debug.LogError(r.name + " has no mesh!");
            }

            WriteMesh(bw, mesh, r.sharedMaterials, true, shared_mats, shared_meshes);
        }

        Debug.Log(bw.BaseStream.Position);

        //static mesh in anim
        var mrs = obj.GetComponentsInChildren<MeshRenderer>();
        bw.Write(mrs.Length);
        for(int i = 0; i < mrs.Length; i++)
        {
            var r = mrs[i];

            WriteString(bw, r.gameObject.name);

            WriteVector3(bw, r.transform.position);
            WriteQuaternion(bw, r.transform.rotation);
            WriteVector3(bw, r.transform.lossyScale);

            WriteMesh(bw, r.GetComponent<MeshFilter>().sharedMesh, r.sharedMaterials, false, shared_mats, shared_meshes);

            string parent = "";
            foreach(var j in bone_names)
            {
                if(j.Value == r.transform.parent)
                {
                    parent = j.Key;
                    break;
                }
            }

            WriteString(bw, parent);
        }

        Debug.Log(bw.BaseStream.Position);

        bw.Close();

        Debug.Log("export anim " + obj.name + " done.");
    }
    
    static void WriteVector2(BinaryWriter bw, Vector2 v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
    }

    static void WriteVector3(BinaryWriter bw, Vector3 v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
        bw.Write(v.z);
    }

    static void WriteVector4(BinaryWriter bw, Vector4 v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
        bw.Write(v.z);
        bw.Write(v.w);
    }

    static void WriteQuaternion(BinaryWriter bw, Quaternion v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
        bw.Write(v.z);
        bw.Write(v.w);
    }

    static void WriteString(BinaryWriter bw, string v)
    {
        byte[] s = System.Text.Encoding.UTF8.GetBytes(v);
        bw.Write(s.Length);
        bw.Write(s);
    }

    static void WriteColor(BinaryWriter bw, Color v)
    {
        bw.Write(v.r);
        bw.Write(v.g);
        bw.Write(v.b);
        bw.Write(v.a);
    }

    static void WriteMesh(BinaryWriter bw, Mesh mesh, Material[] sharedMaterials, bool skin, Dictionary<string, Material> shared_mats, Dictionary<string, Mesh> shared_meshes)
    {
        bool write_data = false;

        if(mesh == null)
        {
            WriteString(bw, "");
            bw.Write((byte) 0);
        }
        else
        {
            string path = AssetDatabase.GetAssetPath(mesh);
            if(path.Length == 0)
            {
                AssetDatabase.CreateAsset(mesh, "Assets/" + mesh.name + ".asset");
            }
            string mesh_guid = AssetDatabase.AssetPathToGUID(AssetDatabase.GetAssetPath(mesh));
            mesh_guid += mesh.name;//maybe multiple meshes in one file hase same guid, so append the mesh name

            WriteString(bw, mesh_guid);

            if(shared_meshes.ContainsKey(mesh_guid))
            {
                bw.Write((byte) 0);
            }
            else
            {
                bw.Write((byte) 1);
                shared_meshes.Add(mesh_guid, mesh);
                write_data = true;
            }
        }
        
        if(write_data)
        {
            //vertices
            bw.Write(mesh.vertices.Length);
            for(int i = 0; i < mesh.vertices.Length; i++)
            {
                WriteVector3(bw, mesh.vertices[i]);

                Vector3 normal = new Vector3(0, 0, 0);
                if(mesh.normals.Length > 0)
                {
                    normal = mesh.normals[i];
                }
                WriteVector3(bw, normal);

                Vector4 tangent = new Vector4(0, 0, 0, 0);
                if(mesh.tangents.Length > 0)
                {
                    tangent = mesh.tangents[i];
                }
                WriteVector4(bw, tangent);

                if(skin)
                {
                    Vector4 weight = new Vector4(0, 0, 0, 0);
                    Vector4 index = new Vector4(0, 0, 0, 0);

                    if(mesh.boneWeights.Length > 0)
                    {
                        weight = new Vector4(
                            mesh.boneWeights[i].weight0,
                            mesh.boneWeights[i].weight1,
                            mesh.boneWeights[i].weight2,
                            mesh.boneWeights[i].weight3
                            );

                        index = new Vector4(
                            (float) mesh.boneWeights[i].boneIndex0,
                            (float) mesh.boneWeights[i].boneIndex1,
                            (float) mesh.boneWeights[i].boneIndex2,
                            (float) mesh.boneWeights[i].boneIndex3
                            );
                    }

                    WriteVector4(bw, weight);
                    WriteVector4(bw, index);
                }

                Vector2 uv1 = new Vector2(0, 0);
                if(mesh.uv.Length > 0)
                {
                    uv1 = mesh.uv[i];
                    uv1.y = 1.0f - uv1.y;
                }
                WriteVector2(bw, uv1);

                if(!skin)
                {
                    Vector2 uv2 = new Vector2(0, 0);
                    if(mesh.uv2.Length > 0)
                    {
                        uv2 = mesh.uv2[i];
                    }
                    WriteVector2(bw, uv2);
                }
            }

            //indices
            bw.Write(mesh.subMeshCount);
            for(int i = 0; i < mesh.subMeshCount; i++)
            {
                int[] sub = mesh.GetTriangles(i);

                bw.Write(sub.Length);
                for(int j = 0; j < sub.Length; j++)
                {
                    bw.Write((ushort) sub[j]);
                }
            }

            if(skin)
            {
                bw.Write(mesh.bindposes.Length);
                for(int i = 0; i < mesh.bindposes.Length; i++)
                {
                    for(int j = 0; j < 16; j++)
                    {
                        bw.Write(mesh.bindposes[i][j / 4, j % 4]);
                    }
                }
            }
        }
        
        bw.Write(sharedMaterials.Length);
        for(int i = 0; i < sharedMaterials.Length; i++)
        {
            WriteMaterial(bw, sharedMaterials[i], shared_mats);
        }
    }

    static void WriteLightmap(BinaryWriter bw, Renderer render)
    {
        bw.Write(render.lightmapIndex);
        WriteVector4(bw, render.lightmapScaleOffset);
    }

    static void WriteMaterial(BinaryWriter bw, Material mat, Dictionary<string, Material> shared_mats)
    {
        string path = AssetDatabase.GetAssetPath(mat);
        if(path.Length == 0)
        {
            AssetDatabase.CreateAsset(mat, "Assets/" + mat.name + ".mat");
        }
        string mat_guid = AssetDatabase.AssetPathToGUID(AssetDatabase.GetAssetPath(mat));

        WriteString(bw, mat_guid);
        
        if(shared_mats.ContainsKey(mat_guid))
        {
            bw.Write((byte) 0);
            return;
        }
        else
        {
            bw.Write((byte) 1);
            shared_mats.Add(mat_guid, mat);
        }

        WriteString(bw, mat.name);

        Shader shader = mat.shader;
        WriteString(bw, shader.name);

        int property_count = ShaderUtil.GetPropertyCount(shader);

        bw.Write(property_count);
        for(int i = 0; i < property_count; i++)
        {
            ShaderUtil.ShaderPropertyType type = ShaderUtil.GetPropertyType(shader, i);
            bw.Write((int) type);

            string propertyName = ShaderUtil.GetPropertyName(shader, i);
            WriteString(bw, propertyName);

            if(type == ShaderUtil.ShaderPropertyType.TexEnv)
            {
                Texture tex = mat.GetTexture(propertyName);
                string texPath = AssetDatabase.GetAssetPath(tex);
                string tex_name = null;
                //tex_name = texPath.Replace("\\", "/").Substring(tex_name.LastIndexOf('/') + 1);
                tex_name = AssetDatabase.AssetPathToGUID(texPath);
                if(tex != null)
                {
                    tex_name += ((int) tex.wrapMode).ToString();
                }

                WriteString(bw, tex_name);

                if(tex != null && tex is Texture2D)
                {
                    var bytes = (tex as Texture2D).EncodeToPNG();
                    if(bytes != null)
                    {
                        File.WriteAllBytes(Application.dataPath + "/../mesh_texture_out/" + tex_name, bytes);
                    }
                    else
                    {
                        Debug.Log(texPath);
                    }
                }
            }
            else if(type == ShaderUtil.ShaderPropertyType.Color)
            {
                Color col = mat.GetColor(propertyName);

                WriteColor(bw, col);
            }
            else if(
                type == ShaderUtil.ShaderPropertyType.Range ||
                type == ShaderUtil.ShaderPropertyType.Float)
            {
                float v = mat.GetFloat(propertyName);

                bw.Write(v);
            }
            else if(type == ShaderUtil.ShaderPropertyType.Vector)
            {
                Vector4 v = mat.GetVector(propertyName);

                WriteVector4(bw, v);
            }
        }
    }
}

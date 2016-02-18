using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

public class MeshSpliter
{
    //[MenuItem("Geart3D/Split mesh 4")]
    static void Split4()
    {
        if(Selection.gameObjects == null || Selection.gameObjects.Length != 1)
        {
            return;
        }

        GameObject obj = Selection.activeGameObject;
        var renderer = obj.GetComponent<MeshRenderer>();
        if(renderer == null)
        {
            return;
        }

        Split(renderer, 4);

        Debug.Log("split done!");
    }

    static void Split(MeshRenderer renderer, int count)
    {
        var mesh = renderer.GetComponent<MeshFilter>().sharedMesh;
        var ts = mesh.triangles;
        List<int>[] ins = new List<int>[count];
        for(int i=0; i<ts.Length;)
        {
            int index = ts[i];

            for(int j=0; j<count; j++)
            {
                if((mesh.colors[index].r >= j * 1.0 / count) && (mesh.colors[index].r < (j + 1) * 1.0 / count))
                {
                    if(ins[j] == null)
                    {
                        ins[j] = new List<int>();
                    }

                    ins[j].Add(ts[i]);
                    ins[j].Add(ts[i+1]);
                    ins[j].Add(ts[i+2]);
                    break;
                }
            }

            i += 3;
        }

        var mesh_new = new Mesh();
        mesh_new.name = "mesh_split";
        mesh_new.vertices = mesh.vertices;
        mesh_new.uv = mesh.uv;
        mesh_new.uv2 = mesh.uv2;
        mesh_new.normals = mesh.normals;
        mesh_new.subMeshCount = count;
        int sub = 0;
        for(int i=0; i<count; i++)
        {
            if(ins[i] != null)
            {
                mesh_new.SetTriangles(ins[i].ToArray(), sub++);
            }
        }
        
        var mf = new GameObject("split mesh").AddComponent<MeshFilter>();
        mf.sharedMesh = mesh_new;

        var mr = mf.gameObject.AddComponent<MeshRenderer>();
        Material[] mats = new Material[count];
        for(int i=0; i<count; i++)
        {
            mats[i] = new Material(Shader.Find("Diffuse"));
            mats[i].name = "mat_" + i.ToString();
        }
        mr.sharedMaterials = mats;
    }
}

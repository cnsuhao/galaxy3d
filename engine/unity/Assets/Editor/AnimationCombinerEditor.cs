using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

public class AnimationCombinerEditor
{
    //[MenuItem("Geart3D/Combine renderers")]
    static void CombineRenderers()
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

        var renderers = anim.GetComponentsInChildren<SkinnedMeshRenderer>();
        var renderer = new GameObject("renderer").AddComponent<SkinnedMeshRenderer>();
        var mesh = new Mesh();
        mesh.name = "combined";
        CombineInstance[] instances = new CombineInstance[renderers.Length];
        List<Transform> bones = new List<Transform>();
        Texture2D tex = new Texture2D(1, 1, TextureFormat.RGB24, true);
        Texture2D[] textures = new Texture2D[renderers.Length];

        for(int i=0; i<renderers.Length; i++)
        {
            instances[i] = new CombineInstance();
            instances[i].mesh = renderers[i].sharedMesh;
            instances[i].subMeshIndex = 0;
            instances[i].transform = Matrix4x4.identity;

            bones.AddRange(renderers[i].bones);
            textures[i] = renderers[i].sharedMaterial.mainTexture as Texture2D;
        }

        mesh.CombineMeshes(instances);
        renderer.sharedMesh = mesh;
        renderer.transform.parent = anim.transform;
        renderer.transform.localPosition = Vector3.zero;
        renderer.transform.localRotation = Quaternion.identity;
        renderer.transform.localScale = Vector3.one;
        renderer.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.On;
        renderer.bones = bones.ToArray();

        Material mat = new Material(Shader.Find("Diffuse"));
        renderer.sharedMaterial = mat;

        tex.PackTextures(textures, 1);
        mat.mainTexture = tex;
    }

	//[MenuItem("Geart3D/Combine animations")]
	static void CombineAnimations()
    {
	    if(Selection.gameObjects == null || Selection.gameObjects.Length != 1)
        {
            return;
        }

        GameObject obj = Selection.activeGameObject;
        var combiner = obj.GetComponent<AnimationCombiner>();
        if(combiner == null)
        {
            return;
        }

        for(int i=0; i<combiner.meshes.Length; i++)
        {
            var mesh = Object.Instantiate<GameObject>(combiner.meshes[i].gameObject).GetComponent<SkinnedMeshRenderer>();
            mesh.transform.parent = combiner.anim.transform;
            mesh.name = combiner.meshes[i].name;
            mesh.transform.localPosition = combiner.meshes[i].transform.localPosition;
            mesh.transform.localRotation = combiner.meshes[i].transform.localRotation;
            mesh.transform.localScale = combiner.meshes[i].transform.localScale;

            Transform[] bones = new Transform[mesh.bones.Length];
            for(int j=0; j<mesh.bones.Length; j++)
            {
                string path = GetBonePath(mesh.bones[j]);

                bones[j] = combiner.anim.transform.Find(path);
            }

            mesh.bones = bones;
            mesh.rootBone = combiner.anim.transform.Find(GetBonePath(mesh.rootBone));
        }
	}

    static string GetBonePath(Transform t)
    {
        var str = new System.Text.StringBuilder();

        str.Append(t.name);
        Transform p = t.parent;
        while(p != null && p.GetComponent<Animation>() == null)
        {
            str.Insert(0, "/");
            str.Insert(0, p.name);
            p = p.parent;
        }

        return str.ToString();
    }
}
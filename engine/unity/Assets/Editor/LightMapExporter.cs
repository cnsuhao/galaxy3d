using UnityEditor;
using UnityEngine;

public class LightmapExporter
{
    [MenuItem("Geart3D/Export Lightmap")]
    public static void Export()
    {
        if(Selection.objects == null || Selection.objects.Length != 1)
        {
            return;
        }

        Texture2D tex = Selection.activeObject as Texture2D;
        if(tex != null)
        {
            var bytes = tex.EncodeToPNG();
            System.IO.File.WriteAllBytes(Application.dataPath + "/" + tex.name + ".png", bytes);

            Debug.Log("lightmap export done:" + tex.name);
        }
    }
}
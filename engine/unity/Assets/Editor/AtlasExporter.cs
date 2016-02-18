using UnityEditor;
using UnityEngine;
using LitJson;

public class AtlasExporter
{
    [MenuItem("Geart3D/Export Atlas Json")]
    public static void Export()
    {
        if(Selection.objects == null || Selection.objects.Length != 1)
        {
            return;
        }

        UIAtlas atlas = Selection.activeGameObject.GetComponent<UIAtlas>();
        if(atlas != null)
        {
            var js = new JsonData();
            var frames = new JsonData();
            var meta = new JsonData();
            var tex = AssetDatabase.GetAssetPath(atlas.texture);
            tex = tex.Substring(tex.LastIndexOf('/') + 1);
            meta["image"] = tex;

            for(int i=0; i<atlas.spriteList.Count; i++)
            {
                var s = atlas.spriteList[i];

                var sprite = new JsonData();
                sprite["filename"] = s.name;
                var frame = new JsonData();
                frame["x"] = s.x;
                frame["y"] = s.y;
                frame["w"] = s.width;
                frame["h"] = s.height;
                sprite["frame"] = frame;
                var border = new JsonData();
                border["x"] = s.borderLeft;
                border["y"] = s.borderTop;
                border["z"] = s.borderRight;
                border["w"] = s.borderBottom;
                sprite["border"] = border;

                frames.Add(sprite);
            }

            js["meta"] = meta;
            js["frames"] = frames;

            System.IO.File.WriteAllText(Application.dataPath + "/" + atlas.name + ".json", js.ToJson());
        }
    }
}
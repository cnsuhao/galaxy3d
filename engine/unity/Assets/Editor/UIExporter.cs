using UnityEditor;
using UnityEngine;
using LitJson;

public class UIExporter
{
    [MenuItem("Geart3D/Export UI Json")]
    static void Export()
    {
        if(Selection.objects == null || Selection.objects.Length != 1)
        {
            return;
        }

        var obj = Selection.activeGameObject;
        var js = WriteNode(obj.transform);

        System.IO.File.WriteAllText(Application.dataPath + "/" + obj.name + ".json", js.ToJson());

        Debug.Log("export ui " + obj.name + " done.");
    }

    static JsonData WriteNode(Transform t)
    {
        var transform = new JsonData();
        var rot = new JsonData();
        transform["name"] = t.name;
        transform["local_position"] = WriteVector3(t.localPosition);
        rot["x"] = t.localRotation.x;
        rot["y"] = t.localRotation.y;
        rot["z"] = t.localRotation.z;
        rot["w"] = t.localRotation.w;
        transform["local_rotation"] = rot;
        transform["local_scale"] = WriteVector3(t.localScale);
        transform["active"] = t.gameObject.activeSelf;

        var components = new JsonData();
        components.SetJsonType(JsonType.Array);

        var sprite = t.GetComponent<UISprite>();
        if(sprite != null)
        {
            var com = new JsonData();
            com["type"] = "Sprite";
            com["sprite"] = sprite.spriteName;
            com["sprite_atlas"] = sprite.atlas.name;
            com["sprite_type"] = sprite.type == UIBasicSprite.Type.Advanced ? 1 : (int) sprite.type;
            com["sprite_flip"] = (int) sprite.flip;
            com["sprite_fill_dir"] = (int) sprite.fillDirection;
            com["sprite_fill_amount"] = sprite.fillAmount;
            com["sprite_fill_invert"] = sprite.invert;
            com["color"] = WriteColor(sprite.color);
            com["pivot"] = (int) sprite.pivot;
            com["depth"] = sprite.depth;
            com["width"] = sprite.width;
            com["height"] = sprite.height;
            com["enable"] = sprite.enabled;

            components.Add(com);
        }

        var collider = t.GetComponent<BoxCollider>();
        if(collider)
        {
            var com = new JsonData();
            com["type"] = "BoxCollider";
            com["center"] = WriteVector3(collider.center);
            com["size"] = WriteVector3(collider.size);

            components.Add(com);
        }

        var label = t.GetComponent<UILabel>();
        if(label)
        {
            var com = new JsonData();
            com["type"] = "Label";
            if(label.trueTypeFont != null)
            {
                com["font"] = label.trueTypeFont.name;
            }
            else
            {
                com["font"] = "";
            }
            com["font_size"] = label.fontSize;
            com["font_style"] = label.fontStyle.ToString();
            com["text"] = label.text;
            com["overflow"] = label.overflowMethod.ToString();
            com["alignment"] = label.alignment == NGUIText.Alignment.Justified ? 0 : (int) label.alignment;
            com["effect_style"] = label.effectStyle.ToString();
            com["effect_color"] = WriteColor(label.effectColor);
            com["space_x"] = label.spacingX;
            com["space_y"] = label.spacingY;
            com["max_line"] = label.maxLineCount;
            com["rich"] = label.supportEncoding;
            com["color"] = WriteColor(label.color);
            com["pivot"] = (int) label.pivot;
            com["depth"] = label.depth;
            com["width"] = label.width;
            com["height"] = label.height;

            components.Add(com);
        }

        var panel = t.GetComponent<UIPanel>();
        if(panel)
        {
            bool clip = (panel.clipping == UIDrawCall.Clipping.SoftClip);
            var com = new JsonData();
            com["type"] = "Panel";
            com["depth"] = panel.depth;
            com["clip"] = clip;
            if(clip)
            {
                com["clip_center_x"] = panel.baseClipRegion.x;
                com["clip_center_y"] = panel.baseClipRegion.y;
                com["clip_size_w"] = panel.baseClipRegion.z;
                com["clip_size_h"] = panel.baseClipRegion.w;
                com["clip_soft_x"] = panel.clipSoftness.x;
                com["clip_soft_y"] = panel.clipSoftness.y;
            }

            components.Add(com);
        }

        transform["components"] = components;

        var children = new JsonData();
        children.SetJsonType(JsonType.Array);
        int child_count = t.childCount;
        for(int i=0; i<child_count; i++)
        {
            children.Add(WriteNode(t.GetChild(i)));
        }

        transform["children"] = children;

        return transform;
    }

    static JsonData WriteColor(Color c)
    {
        var json = new JsonData();
        json["r"] = c.r;
        json["g"] = c.g;
        json["b"] = c.b;
        json["a"] = c.a;

        return json;
    }

    static JsonData WriteVector3(Vector3 v)
    {
        var json = new JsonData();
        json["x"] = v.x;
        json["y"] = v.y;
        json["z"] = v.z;

        return json;
    }
}
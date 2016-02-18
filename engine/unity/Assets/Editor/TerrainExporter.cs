using UnityEditor;
using UnityEngine;

public class TerrainExporter : MonoBehaviour {

	[MenuItem("Geart3D/Export Terrain")]
    public static void Export()
    {
        if(Selection.objects == null || Selection.objects.Length != 1)
        {
            return;
        }

        Terrain ter = Selection.activeGameObject.GetComponent<Terrain>();
        if(ter != null)
        {
            TerrainData data = ter.terrainData;
            var heights = data.GetHeights(0, 0, data.heightmapWidth, data.heightmapHeight);
            var alphas = data.GetAlphamaps(0, 0, data.alphamapWidth, data.alphamapHeight);

            byte[] height_map = new byte[data.heightmapWidth * data.heightmapHeight * 2];
            for(int i=0; i<data.heightmapHeight; i++)
            {
                for(int j=0; j<data.heightmapWidth; j++)
                {
                    //float h = ter.SampleHeight(new Vector3(j, 0, i));
                    //float h = data.GetHeight(j, i);
                    //Debug.Log(string.Format("{0} {1} {2}", data.GetHeight(j, i), data.GetInterpolatedHeight(j/500.0f, i), ter.SampleHeight(new Vector3(j, 0, i))));
                    int h = (int) (heights[i, j] * 65535);

                    height_map[(data.heightmapHeight - 1 - i) * data.heightmapWidth * 2 + j * 2 + 1] = (byte) ((h & 0xff00) >> 8);
                    height_map[(data.heightmapHeight - 1 - i) * data.heightmapWidth * 2 + j * 2] = (byte) (h & 0xff);
                }
            }

            System.IO.File.WriteAllBytes(Application.dataPath + "/" + ter.name + ".raw", height_map);


            int layers = data.alphamapLayers;
            Color[] colors = new Color[data.alphamapHeight * data.alphamapWidth];
            for(int i=0; i<data.alphamapHeight; i++)
            {
                for(int j=0; j<data.alphamapWidth; j++)
                {
                    Color c = new Color();

                    float[] rgba = new float[4];
                    for(int k = 0; k < layers && k < 4; k++)
                    {
                        float alpha = alphas[j, i, k];

                        rgba[k] = alpha;
                    }

                    c.r = rgba[0];
                    c.g = rgba[1];
                    c.b = rgba[2];
                    c.a = rgba[3];

                    colors[(j) * data.alphamapWidth + (i)] = c;
                }
            }

            Texture2D tex = new Texture2D(data.alphamapWidth, data.alphamapHeight, TextureFormat.RGBA32, false);
            tex.SetPixels(colors);
            System.IO.File.WriteAllBytes(Application.dataPath + "/" + ter.name + ".png", tex.EncodeToPNG());

            Debug.Log("terrain export done:" + ter.name);
        }
    }
}

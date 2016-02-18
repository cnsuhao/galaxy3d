using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.IO;

public class NavMeshExporter
{
    class Edge
    {
        public int vertex_left;
        public int vertex_right;
        public int neighbor;

        public Edge(int left, int right)
        {
            vertex_left = left;
            vertex_right = right;
            neighbor = -1;
        }
    }

    class Triangle
    {
        public Edge[] edges;

        public Triangle()
        {
            edges = new Edge[3];
        }
    }

    [MenuItem("Geart3D/Export nav mesh")]
    static void Export()
    {
        var ts = NavMesh.CalculateTriangulation();

        Mesh mesh = new Mesh();
        mesh.name = "navmesh";
        mesh.vertices = ts.vertices;
        mesh.triangles = ts.indices;

        AssetDatabase.CreateAsset(mesh, "Assets/navmesh.asset");

        Triangle[] triangles = new Triangle[ts.indices.Length / 3];
        for(int i=0; i<triangles.Length; i++)
        {
            Triangle t = new Triangle();

            t.edges[0] = new Edge(ts.indices[i * 3 + 0], ts.indices[i * 3 + 1]);
            t.edges[1] = new Edge(ts.indices[i * 3 + 1], ts.indices[i * 3 + 2]);
            t.edges[2] = new Edge(ts.indices[i * 3 + 2], ts.indices[i * 3 + 0]);

            triangles[i] = t;
        }

        int neighbor_count = 0;

        for(int i=0; i<triangles.Length; i++)
        {
            for(int j=0; j<3; j++)
            {
                var e1 = triangles[i].edges[j];

                for(int k=0; k<triangles.Length; k++)
                {
                    bool found_neighbor = false;

                    if(triangles[k] != triangles[i])
                    {
                        for(int m=0; m<3; m++)
                        {
                            var e2 = triangles[k].edges[m];

                            Vector3 e1_left = ts.vertices[e1.vertex_left];
                            Vector3 e1_right = ts.vertices[e1.vertex_right];
                            Vector3 e2_left = ts.vertices[e2.vertex_left];
                            Vector3 e2_right = ts.vertices[e2.vertex_right];

                            if( e1_left == e2_right &&
                                e1_right == e2_left)
                            {
                                e1.neighbor = k;
                                e2.neighbor = i;

                                found_neighbor = true;
                                neighbor_count++;
                                break;
                            }
                        }
                    }

                    if(found_neighbor)
                    {
                        break;
                    }
                }
            }
        }

        string path = Application.dataPath + "/navmesh.nav";
        BinaryWriter bw = new BinaryWriter(File.Open(path, FileMode.Create));

        bw.Write(ts.vertices.Length);
        for(int i=0; i<ts.vertices.Length; i++)
        {
            bw.Write(ts.vertices[i].x);
            bw.Write(ts.vertices[i].y);
            bw.Write(ts.vertices[i].z);
        }

        bw.Write(ts.indices.Length);
        for(int i=0; i<ts.indices.Length; i++)
        {
            bw.Write(ts.indices[i]);
        }
        
        bw.Write(triangles.Length);
        for(int i=0; i<triangles.Length; i++)
        {
            for(int j=0; j<3; j++)
            {
                var e = triangles[i].edges[j];

                bw.Write(e.vertex_left);
                bw.Write(e.vertex_right);
                bw.Write(e.neighbor);
            }
        }

        bw.Close();

        Debug.Log("export navmesh done with neighbor count " + neighbor_count + ".");
    }
}

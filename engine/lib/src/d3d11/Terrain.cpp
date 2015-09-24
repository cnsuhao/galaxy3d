#include "Terrain.h"
#include "GTFile.h"
#include "Texture2D.h"
#include "GTString.h"
#include "Transform.h"

#define GEO_PATCH_SIZE 17
#define GEO_LEVEL_MAX 3
#define GEO_DISTANCE_THRESHOLD 50.0f
#define GEO_DELTA_THRESHOLD 1.5f

namespace Galaxy3D
{
	Terrain::Terrain():
		m_vertex_buffer(0),
		m_index_buffer(0),
        m_geo_patches(0),
        m_geo_patch_count_per_side(0),
        m_use_geo(false)
	{
	}

	Terrain::~Terrain()
	{
		DeleteBuffers();

		if(m_geo_patches != 0)
		{
			delete [] m_geo_patches;
		}
	}

	void Terrain::LoadData(
			int size_heightmap,
			float xz_unit,
			float y_unit,
			const std::string &file_heightmap,
			const std::string &file_alphamap,
			const std::vector<std::string> &file_textures,
			float texture_size)
	{
        m_map_size = size_heightmap;
		m_xz_unit = xz_unit;

        int file_size;
        char *buffer = (char *) GTFile::ReadAllBytes(file_heightmap, &file_size);
        
        if(buffer == NULL)
        {
            return;
        }

        do
        {
            int point_cout = m_map_size * m_map_size;
            if(point_cout * 2 != file_size)
            {
                break;
            }

            m_vertices.resize(point_cout);
            m_indices.resize((m_map_size - 1) * (m_map_size - 1) * 2 * 3);

            float uv_unit = 1 / texture_size;
            int k = 0;
            for(int i=0; i<m_map_size; i++)
            {
                for(int j=0; j<m_map_size; j++)
                {
                    unsigned short ushort = *((unsigned short *) &buffer[(i * m_map_size + j) * 2]);
                    float y = y_unit * ushort / 65535.0f;
                    float x = j * xz_unit;
                    float z = (m_map_size - 1 - i) * xz_unit;

                    auto &vertex = m_vertices[i * m_map_size + j];
                    vertex.POSITION = Vector3(x, y, z);
                    vertex.TEXCOORD0 = Vector2(x, z) * uv_unit;
                    vertex.TEXCOORD1 = Vector2(x, z) * (1.0f / ((m_map_size - 1) * xz_unit));

                    if(i < m_map_size - 1 && j < m_map_size - 1)
                    {
                        m_indices[k++] = i * m_map_size + j;
                        m_indices[k++] = (i + 1) * m_map_size + (j + 1);
                        m_indices[k++] = (i + 1) * m_map_size + j;

                        m_indices[k++] = i * m_map_size + j;
                        m_indices[k++] = i * m_map_size + (j + 1);
                        m_indices[k++] = (i + 1) * m_map_size + (j + 1);
                    }
                }
            }

            auto tex = Texture2D::LoadFromFile(file_alphamap, FilterMode::Bilinear, TextureWrapMode::Clamp);
            m_shared_material = Material::Create("Terrain/Diffuse");
            m_shared_material->SetVector("TerrainSize", Vector4(1.0f / (m_map_size * xz_unit), 0, 0, 0));
            m_shared_material->SetTexture("AlphaMap", tex);
            for(size_t i=0; i<4 && i<file_textures.size(); i++)
            {
                tex = Texture2D::LoadFromFile(file_textures[i], FilterMode::Bilinear, TextureWrapMode::Repeat, true);
                m_shared_material->SetTexture(GTString::Format("Layer_%d", i).str, tex);
            }

            CalculateNormals();

            m_geo_patch_count_per_side = (m_map_size - 1)/(GEO_PATCH_SIZE - 1);
            m_geo_patches = new GeoMipmapPatch[m_geo_patch_count_per_side * m_geo_patch_count_per_side];

            for(int i=0; i<m_geo_patch_count_per_side; i++)
            {
                for(int j=0; j<m_geo_patch_count_per_side; j++)
                {
                    GeoMipmapPatch &patch = m_geo_patches[i * m_geo_patch_count_per_side + j];

                    patch.level = GEO_LEVEL_MAX;
                    patch.visible = true;

                    int m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 2;
                    int n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 2;
                    Vector3 center_pos = m_vertices[m * m_map_size + n].POSITION;

                    float max = 0;

                    m = i * (GEO_PATCH_SIZE - 1) + 0;
                    n = j * (GEO_PATCH_SIZE - 1) + 0;
                    Vector3 corner_pos = m_vertices[m * m_map_size + n].POSITION;
                    m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 4;
                    n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 4;
                    Vector3 corner_center_pos = m_vertices[m * m_map_size + n].POSITION;
                    Vector3 corner_lerp = (center_pos + corner_pos) * 0.5f;
                    Vector3 left_top_delta = corner_center_pos - corner_lerp;
                    max = Mathf::Max((corner_pos - center_pos).SqrMagnitude(), max);

                    m = i * (GEO_PATCH_SIZE - 1) + 0;
                    n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1);
                    corner_pos = m_vertices[m * m_map_size + n].POSITION;
                    m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 4;
                    n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) * 3 / 4;
                    corner_center_pos = m_vertices[m * m_map_size + n].POSITION;
                    corner_lerp = (center_pos + corner_pos) * 0.5f;
                    left_top_delta += corner_center_pos - corner_lerp;
                    max = Mathf::Max((corner_pos - center_pos).SqrMagnitude(), max);

                    m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1);
                    n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1);
                    corner_pos = m_vertices[m * m_map_size + n].POSITION;
                    m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) * 3 / 4;
                    n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) * 3 / 4;
                    corner_center_pos = m_vertices[m * m_map_size + n].POSITION;
                    corner_lerp = (center_pos + corner_pos) * 0.5f;
                    left_top_delta += corner_center_pos - corner_lerp;
                    max = Mathf::Max((corner_pos - center_pos).SqrMagnitude(), max);

                    m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1);
                    n = j * (GEO_PATCH_SIZE - 1) + 0;
                    corner_pos = m_vertices[m * m_map_size + n].POSITION;
                    m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) * 3 / 4;
                    n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 4;
                    corner_center_pos = m_vertices[m * m_map_size + n].POSITION;
                    corner_lerp = (center_pos + corner_pos) * 0.5f;
                    left_top_delta += corner_center_pos - corner_lerp;
                    max = Mathf::Max((corner_pos - center_pos).SqrMagnitude(), max);

                    patch.delta = fabs(left_top_delta.y) * 0.25f;
                    patch.center = center_pos;
                    patch.bounding_sphere_radius = sqrt(max);
                }
            }
        } while(false);

        free(buffer);
	}

	float Terrain::GetHeight(const Vector3 &world_pos) const
	{
		Vector3 pos = GetTransform()->InverseTransformPoint(world_pos);
		int x = int(pos.x / m_xz_unit);
		int y = int((m_map_size - 1 ) - pos.z / m_xz_unit);
		
		if(x > 0 && x < m_map_size && y > 0 && y < m_map_size)
		{
			//近似取点
			return m_vertices[y * m_map_size + x].POSITION.y;
		}

		return 0;
	}

	void Terrain::UpdateGeoMipmap()
	{
		Vector3 cam_pos_in_model = GetTransform()->InverseTransformPoint(m_camera->GetTransform()->GetPosition());
		Matrix4x4 wvp = m_camera->GetViewProjectionMatrix() * GetTransform()->GetLocalToWorldMatrix();
		FrustumBounds frustum(wvp);
		
		for(int i=0; i<m_geo_patch_count_per_side; i++)
		{
			for(int j=0; j<m_geo_patch_count_per_side; j++)
			{
				GeoMipmapPatch &patch = m_geo_patches[i * m_geo_patch_count_per_side + j];

				int m = i * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 2;
				int n = j * (GEO_PATCH_SIZE - 1) + (GEO_PATCH_SIZE - 1) / 2;
				Vector3 center_pos = m_vertices[m * m_map_size + n].POSITION;

				patch.visible = frustum.ContainsSphere(center_pos, patch.bounding_sphere_radius) >= 0;

				if(!patch.visible)
				{
					continue;
				}
				
				float distance = (cam_pos_in_model - center_pos).Magnitude();
				
				patch.level = (int) (distance / (GEO_DISTANCE_THRESHOLD * Mathf::Max(1.0f, patch.delta * GEO_DELTA_THRESHOLD)));
				
				patch.level = Mathf::Min(GEO_LEVEL_MAX, patch.level);
				

				if(j > 0 && m_geo_patches[i * m_geo_patch_count_per_side + j - 1].level - patch.level > 1)
				{
					patch.level = m_geo_patches[i * m_geo_patch_count_per_side + j - 1].level - 1;
				}
				else if(j > 0 && m_geo_patches[i * m_geo_patch_count_per_side + j - 1].level - patch.level < -1)
				{
					patch.level = m_geo_patches[i * m_geo_patch_count_per_side + j - 1].level + 1;
				}

				if(i > 0 && m_geo_patches[(i - 1) * m_geo_patch_count_per_side + j].level - patch.level > 1)
				{
					patch.level = m_geo_patches[(i - 1) * m_geo_patch_count_per_side + j].level - 1;
				}
				else if(i > 0 && m_geo_patches[(i - 1) * m_geo_patch_count_per_side + j].level - patch.level < -1)
				{
					patch.level = m_geo_patches[(i - 1) * m_geo_patch_count_per_side + j].level + 1;
				}
			}
		}

		GeoMipmapNeighbor patch_neighbor;
		GeoMipmapNeighbor fan_neighbor;
        m_geo_indices.clear();

		for(int i=0; i<m_geo_patch_count_per_side; i++)
		{
			for(int j=0; j<m_geo_patch_count_per_side; j++)
			{
				GeoMipmapPatch &patch = m_geo_patches[i * m_geo_patch_count_per_side + j];

				if(!patch.visible)
				{
					continue;
				}

				patch_neighbor.left = (j == 0) || (m_geo_patches[i * m_geo_patch_count_per_side + (j - 1)].level <= patch.level);
				patch_neighbor.top = (i == 0) || (m_geo_patches[(i - 1) * m_geo_patch_count_per_side + j].level <= patch.level);
				patch_neighbor.right = (j == m_geo_patch_count_per_side - 1) || (m_geo_patches[i * m_geo_patch_count_per_side + (j + 1)].level <= patch.level);
				patch_neighbor.bottom = (i == m_geo_patch_count_per_side - 1) || (m_geo_patches[(i + 1) * m_geo_patch_count_per_side + j].level <= patch.level);
			
				int lod = patch.level;
				int size = 1;
				while(lod-- >= 0)
				{
					size <<= 1;
				}

				int half = size >> 1;

				for(int m = half; m + half < GEO_PATCH_SIZE + 1; m += size)
				{
					for(int n = half; n + half < GEO_PATCH_SIZE + 1; n += size)
					{
						if(n == half)
						{
							fan_neighbor.left = patch_neighbor.left;
						}
						else
						{
							fan_neighbor.left = true;
						}

						if(m == half)
						{
							fan_neighbor.top = patch_neighbor.top;
						}
						else
						{
							fan_neighbor.top = true;
						}

						if(n >= GEO_PATCH_SIZE - 1 - half)
						{
							fan_neighbor.right = patch_neighbor.right;
						}
						else
						{
							fan_neighbor.right = true;
						}

						if(m >= GEO_PATCH_SIZE - 1 - half)
						{
							fan_neighbor.bottom = patch_neighbor.bottom;
						}
						else
						{
							fan_neighbor.bottom = true;
						}

						int x = j * (GEO_PATCH_SIZE - 1) + n;
						int z = i * (GEO_PATCH_SIZE - 1) + m;

						if(fan_neighbor.left)
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z + half) * m_map_size + x - half);
                            m_geo_indices.push_back(z * m_map_size + x - half);

                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back(z * m_map_size + x - half);
                            m_geo_indices.push_back((z - half) * m_map_size + x - half);
						}
						else
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z + half) * m_map_size + x - half);
                            m_geo_indices.push_back((z - half) * m_map_size + x - half);
						}

						if(fan_neighbor.top)
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z - half) * m_map_size + x - half);
                            m_geo_indices.push_back((z - half) * m_map_size + x);

                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z - half) * m_map_size + x);
                            m_geo_indices.push_back((z - half) * m_map_size + x + half);
						}
						else
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z - half) * m_map_size + x - half);
                            m_geo_indices.push_back((z - half) * m_map_size + x + half);
						}

						if(fan_neighbor.right)
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z - half) * m_map_size + x + half);
                            m_geo_indices.push_back(z * m_map_size + x + half);

                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back(z * m_map_size + x + half);
                            m_geo_indices.push_back((z + half) * m_map_size + x + half);
						}
						else
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z - half) * m_map_size + x + half);
                            m_geo_indices.push_back((z + half) * m_map_size + x + half);
						}

						if(fan_neighbor.bottom)
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z + half) * m_map_size + x + half);
                            m_geo_indices.push_back((z + half) * m_map_size + x);

                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z + half) * m_map_size + x);
                            m_geo_indices.push_back((z + half) * m_map_size + x - half);
						}
						else
						{
                            m_geo_indices.push_back(z * m_map_size + x);
                            m_geo_indices.push_back((z + half) * m_map_size + x + half);
                            m_geo_indices.push_back((z + half) * m_map_size + x - half);
						}
					}
				}
			}
		}
	}

	void Terrain::CalculateNormals()
	{
		//计算所有三角形法线
		Vector3 *face_normals = new Vector3[(m_map_size - 1) * (m_map_size - 1) * 2];
		Vector3 face_shared[6];
		for(int i=0; i<m_map_size; i++)
		{
			for(int j=0; j<m_map_size; j++)
			{
				if(i < m_map_size - 1 && j < m_map_size - 1)
				{
					Vector3 a = m_vertices[i * m_map_size + j].POSITION;
					Vector3 b = m_vertices[(i + 1) * m_map_size + j].POSITION;
					Vector3 c = m_vertices[(i + 1) * m_map_size + (j + 1)].POSITION;
					Vector3 d = m_vertices[i * m_map_size + (j + 1)].POSITION;

					Vector3 m = (c - a) * (b - a);
					Vector3 n = (d - a) * (c - a);

					face_normals[i * (m_map_size - 1) * 2 + j * 2] = Vector3::Normalize(m);
					face_normals[i * (m_map_size - 1) * 2 + j * 2 + 1] = Vector3::Normalize(n);
				}

				int shared = 0;
				if(i < m_map_size - 1 && j < m_map_size - 1)
				{
					if(j > 0 && i > 0)
					{
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + (j - 1) * 2];
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + (j - 1) * 2 + 1];
					}
					else if(j > 0)
					{
						face_shared[shared++] = face_normals[i * (m_map_size - 1) * 2 + (j - 1) * 2 + 1];
					}
					else if(i > 0)
					{
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + j * 2];
					}
					face_shared[shared++] = face_normals[i * (m_map_size - 1) * 2 + j * 2];
					face_shared[shared++] = face_normals[i * (m_map_size - 1) * 2 + j * 2 + 1];
				}
				else if(j == m_map_size - 1)//right column
				{
					if(i > 0)
					{
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + (j - 1) * 2];
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + (j - 1) * 2 + 1];
					}

					if(i < m_map_size - 1)
					{
						face_shared[shared++] = face_normals[i * (m_map_size - 1) * 2 + (j - 1) * 2 + 1];
					}
				}
				else if(i == m_map_size - 1)//bottom row
				{
					if(j > 0)
					{
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + (j - 1) * 2];
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + (j - 1) * 2 + 1];
					}

					if(j < m_map_size - 1)
					{
						face_shared[shared++] = face_normals[(i - 1) * (m_map_size - 1) * 2 + j * 2 + 1];
					}
				}

				Vector3 normal(0, 0, 0);
				for(int k=0; k<shared; k++)
				{
					normal += face_shared[k];
				}
				normal = normal * (1.0f/shared);

                m_vertices[i * m_map_size + j].NORMAL = Vector3::Normalize(normal);
			}
		}
		delete [] face_normals;
	}

    ID3D11Buffer *Terrain::GetVertexBuffer()
	{
		if(m_vertex_buffer == 0)
		{
            int vertex_size = sizeof(VertexMesh);
            int buffer_size = vertex_size * m_vertices.size();
            char *buffer = (char *) &m_vertices[0];

            auto device = GraphicsDevice::GetInstance()->GetDevice();

            D3D11_BUFFER_DESC dbd;
            ZeroMemory(&dbd, sizeof(dbd));
            dbd.Usage = D3D11_USAGE_IMMUTABLE;
            dbd.ByteWidth = buffer_size;
            dbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            dbd.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA dsd;
            ZeroMemory(&dsd, sizeof(dsd));
            dsd.pSysMem = buffer;
            device->CreateBuffer(&dbd, &dsd, &m_vertex_buffer);
		}

		return m_vertex_buffer;
	}

    ID3D11Buffer *Terrain::GetIndexBuffer()
	{
		if(m_index_buffer == 0)
		{
            auto device = GraphicsDevice::GetInstance()->GetDevice();

            int buffer_size = m_indices.size() * sizeof(int);

            D3D11_BUFFER_DESC dbd;
            ZeroMemory(&dbd, sizeof(dbd));
            dbd.Usage = D3D11_USAGE_DYNAMIC;
            dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            dbd.ByteWidth = buffer_size;
            dbd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA dsd;
            ZeroMemory(&dsd, sizeof(dsd));
            dsd.pSysMem = &m_indices[0];
            device->CreateBuffer(&dbd, &dsd, &m_index_buffer);
		}

        if(m_use_geo)
        {
            UpdateGeoMipmap();
            UpdateIndexBuffer();
        }

		return m_index_buffer;
	}

	void Terrain::UpdateIndexBuffer()
	{
		void *buffer = 0;
		int buffer_size = 0;

		if(m_geo_indices.empty())
		{
			return;
		}

		buffer = (void *) &m_geo_indices[0];
		buffer_size = m_geo_indices.size() * sizeof(int);

		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		D3D11_MAPPED_SUBRESOURCE dms;
		ZeroMemory(&dms, sizeof(dms));
		context->Map(m_index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
		memcpy(dms.pData, buffer, buffer_size);
        context->Unmap(m_index_buffer, 0);
	}

	void Terrain::DeleteBuffers()
    {
        SAFE_RELEASE(m_vertex_buffer);
        SAFE_RELEASE(m_index_buffer);
	}
}

/*
	void Terrain::CreateVertexBuffer()
	{
		if(vertices.empty() || triangles.empty())
		{
			return;
		}
        
		int vertex_size = sizeof(VertexTerrain);
		int buffer_size = vertex_size * vertices.size();
		std::vector<char> buffer(buffer_size);
		char *p = &buffer[0];
        
        fill_vertex_buffer(this, p);
        
        GLuint vertex_buffer;
        
        glGenBuffers(1, &vertex_buffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, &buffer[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        m_vertex_buffer = (void *) vertex_buffer;
	}

	void Terrain::CreateIndexBuffer()
	{
		int buffer_size = triangles.size() * sizeof(int);
		
        GLuint index_buffer;
        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, &triangles[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            
        m_index_buffer = (void *) index_buffer;
	}

	void Terrain::UpdateIndexBuffer()
	{
		void *buffer = 0;
		int buffer_size = 0;

		if(geo_indices.empty())
		{
			return;
		}

		buffer = (void *) &geo_indices[0];
		buffer_size = geo_indices.size() * sizeof(int);

		GLuint index_buffer = (GLuint) m_index_buffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, buffer, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Terrain::DeleteBuffers()
	{
		if(m_vertex_buffer != 0)
		{
			GLuint vertex_buffer = (GLuint) m_vertex_buffer;
            glDeleteBuffers(1, &vertex_buffer);
			m_vertex_buffer = 0;
		}
        
        if(m_index_buffer != 0)
		{
			GLuint index_buffer = (GLuint) m_index_buffer;
            glDeleteBuffers(1, &index_buffer);
			m_index_buffer = 0;
		}
	}
    */
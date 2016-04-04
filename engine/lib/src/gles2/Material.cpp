#include "Material.h"
#include "Debug.h"
#include "Texture2D.h"
#include "Renderer.h"
#include "Guid.h"
#include "RenderTexture.h"
#include "Camera.h"
#include "Cubemap.h"

static const std::string MAIN_TEXTURE_NAME = "_MainTex";
static const std::string MAIN_COLOR_NAME = "_Color";

namespace Galaxy3D
{
	std::shared_ptr<Material> Material::Create(const std::string &shader)
	{
		auto s = Shader::Find(shader);
		if(s)
		{
			return Create(s);
		}
		
		return std::shared_ptr<Material>();
	}

	std::shared_ptr<Material> Material::Create(const std::shared_ptr<Shader> &shader)
	{
		std::shared_ptr<Material> mat(new Material());
		mat->SetShader(shader);
        mat->SetGuid(Guid::NewGuid());

		return mat;
	}

	Material::Material():
		m_render_queue(-1)
	{
        SetMainColor(Color(1, 1, 1, 1));
	}

	void Material::SetShader(const std::shared_ptr<Shader> &shader)
	{
		m_shader = shader;
		SetName(m_shader->GetName());
	}

	void Material::SetRenderQueue(int queue)
	{
		m_render_queue = queue;

        Renderer::SortAllBatches();
	}

    int Material::GetRenderQueue() const
    {
        int queue = m_render_queue;

        if(queue < 0)
        {
            queue = m_shader->GetRenderQueue();
        }

        return queue;
    }

	void Material::SetVector(const std::string &name, const Vector4 &vector)
	{
		m_vectors[name] = vector;
	}

	void Material::SetColor(const std::string &name, const Color &color)
	{
		m_colors[name] = color;
	}

    void Material::SetMainColor(const Color &color)
    {
        m_colors[MAIN_COLOR_NAME] = color;
    }

	void Material::SetMatrix(const std::string &name, const Matrix4x4 &matrix)
	{
		m_matrices[name] = matrix;
	}

	void Material::SetMatrixArray(const std::string &name, const std::vector<Matrix4x4> &matrices)
	{
		m_matrix_arrays[name] = matrices;
	}

    void Material::SetVectorArray(const std::string &name, const std::vector<Vector4> &vectors)
    {
        m_vector_arrays[name] = vectors;
    }

	void Material::SetTexture(const std::string &name, const std::shared_ptr<Texture> &texture)
	{
		m_textures[name] = texture;
	}

	void Material::SetMainTexture(const std::shared_ptr<Texture> &texture)
	{
		m_textures[MAIN_TEXTURE_NAME] = texture;
	}

	std::shared_ptr<Texture> Material::GetMainTexture()
	{
		return GetTexture(MAIN_TEXTURE_NAME);
	}

	std::shared_ptr<Texture> Material::GetTexture(const std::string &name)
	{
		std::shared_ptr<Texture> tex;

		auto find = m_textures.find(name);
		if(find != m_textures.end())
		{
			tex = find->second;
		}

		return tex;
	}

	void Material::Copy(const Material &mat)
	{
		*this = mat;
	}

	void Material::SetVectorDirectlyVS(const std::string &name, const Vector4 &vector, int pass)
	{
	
	}

	void Material::SetMatrixDirectlyVS(const std::string &name, const Matrix4x4 &matrix, int pass)
	{
	
	}

	void Material::SetTextureDirectlyPS(const std::string &name, const std::shared_ptr<Texture> &texture, int pass)
	{
	
	}

	void Material::ReadyPass(int pass)
	{
		auto shader_pass = m_shader->GetPass(pass);

		if(shader_pass->ps == NULL)
        {
            return;
        }

		for(auto &i : shader_pass->ps->textures)
		{
			auto find = m_textures.find(i.first);
			if(find != m_textures.end())
			{
				auto tex = std::dynamic_pointer_cast<Texture2D>(find->second);
				if(tex)
				{
					i.second.texture = tex->GetTexture();
                    continue;
				}

                auto render_texture = std::dynamic_pointer_cast<RenderTexture>(find->second);
                if(render_texture)
                {
                    if(i.first == "_CameraDepthTexture" || i.first == "_ShadowMapTexture")
                    {
                        i.second.texture = render_texture->GetDepthTexture();
                    }
                    else
                    {
                        i.second.texture = render_texture->GetColorTexture();
                    }

                    continue;
                }

                auto cubemap = std::dynamic_pointer_cast<Cubemap>(find->second);
                if(cubemap)
                {
                    i.second.cubemap = cubemap->GetTexture();
                    continue;
                }
			}

			if(i.second.texture == 0)
			{
				i.second.texture = Texture2D::GetDefaultTexture()->GetTexture();
			}
		}
	}

	void Material::ApplyPass(int pass)
	{
		auto shader_pass = m_shader->GetPass(pass);
		auto program = shader_pass->program.program;

		glUseProgram(program);

		for(auto &i : shader_pass->program.cbuffers)
		{
			int slot = i.second.slot;
			if(slot < 0)
			{
				continue;
			}

			{
				auto find = m_vectors.find(i.first);
				if(find != m_vectors.end())
				{
					glUniform4fv(slot, 1, (const GLfloat *) &find->second);
					continue;
				}
			}

			{
				auto find = m_vector_arrays.find(i.first);
				if(find != m_vector_arrays.end())
				{
					glUniform4fv(slot, find->second.size(), (const GLfloat *) &find->second[0]);
					continue;
				}
			}
			
			{
				auto find = m_colors.find(i.first);
				if(find != m_colors.end())
				{
					glUniform4fv(slot, 1, (const GLfloat *) &find->second);
					continue;
				}
			}

			{
				auto find = m_matrices.find(i.first);
				if(find != m_matrices.end())
				{
					glUniformMatrix4fv(slot, 1, GL_FALSE, (const GLfloat *) &find->second);
					continue;
				}
			}

			{
				auto find = m_matrix_arrays.find(i.first);
				if(find != m_matrix_arrays.end())
				{
					glUniformMatrix4fv(slot, find->second.size(), GL_FALSE, (const GLfloat *) &find->second[0]);
					continue;
				}
			}
		}

		int index = 0;
		for(auto &i : shader_pass->ps->textures)
		{
			int slot = i.second.slot;
			if(slot < 0)
			{
				continue;
			}

			glActiveTexture(GL_TEXTURE0 + index);
			if(i.second.cubemap != 0)
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP, i.second.cubemap);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, i.second.texture);
			}
			glUniform1i(slot, index);

			index++;
		}
	}

	void Material::SetZBufferParams(std::shared_ptr<Camera> &cam)
    {
        float cam_far = cam->GetClipFar();
        float cam_near = cam->GetClipNear();

#if defined(WINPC) || defined(WINPHONE)
        float zx = (1.0f - cam_far / cam_near) / 2;
        float zy = (1.0f + cam_far / cam_near) / 2;
#else
        float zx = (1.0f - cam_far / cam_near);
        float zy = (cam_far / cam_near);
#endif

        SetVector("_ZBufferParams", Vector4(zx, zy, zx / cam_far, zy / cam_near));
    }

    void Material::SetProjectionParams(std::shared_ptr<Camera> &cam)
    {
        float cam_far = cam->GetClipFar();
        float cam_near = cam->GetClipNear();

        // x = 1 or -1 (-1 if projection is flipped)
        // y = near plane
        // z = far plane
        // w = 1/far plane
        SetVector("_ProjectionParams", Vector4(1, cam_near, cam_far, 1 / cam_far));
    }

    void Material::SetMainTexTexelSize(const std::shared_ptr<Texture> &tex)
    {
        SetVector("_MainTex_TexelSize", Vector4(1.0f / tex->GetWidth(), 1.0f / tex->GetHeight(), (float) tex->GetWidth(), (float) tex->GetHeight()));
    }
}
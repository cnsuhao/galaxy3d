#include "Material.h"
#include "Debug.h"
#include "Texture2D.h"
#include "Renderer.h"
#include "Guid.h"
#include "RenderTexture.h"

static const std::string MAIN_TEXTURE_NAME = "_MainTex";
static const std::string MAIN_COLOR_NAME = "_MainColor";

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

	static void set_constant_buffer(const std::string &name, void *data, int size, ShaderPass *pass)
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		auto find = pass->vs->cbuffers.find(name);
		if(find != pass->vs->cbuffers.end())
		{
			context->UpdateSubresource(find->second.buffer, 0, NULL, data, size, 0);
		}

		find = pass->ps->cbuffers.find(name);
		if(find != pass->ps->cbuffers.end())
		{
			context->UpdateSubresource(find->second.buffer, 0, NULL, data, size, 0);
		}
	}

    void Material::SetVectorDirectlyVS(const std::string &name, const Vector4 &vector, int pass)
    {
        SetVector(name, vector);

        auto shader_pass = m_shader->GetPass(pass);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        auto find = shader_pass->vs->cbuffers.find(name);
        if(find != shader_pass->vs->cbuffers.end())
        {
            context->UpdateSubresource(find->second.buffer, 0, NULL, &vector, sizeof(Vector4), 0);
        }
    }

    void Material::SetMatrixDirectlyVS(const std::string &name, const Matrix4x4 &matrix, int pass)
    {
        SetMatrix(name, matrix);

        auto shader_pass = m_shader->GetPass(pass);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        auto find = shader_pass->vs->cbuffers.find(name);
        if(find != shader_pass->vs->cbuffers.end())
        {
            context->UpdateSubresource(find->second.buffer, 0, NULL, &matrix, sizeof(Matrix4x4), 0);
        }
    }

    void Material::SetTextureDirectlyPS(const std::string &name, const std::shared_ptr<Texture> &texture, int pass)
    {
        SetTexture(name, texture);

        auto shader_pass = m_shader->GetPass(pass);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        auto find = shader_pass->ps->textures.find(name);
        if(find != shader_pass->ps->textures.end())
        {
            auto tex = std::dynamic_pointer_cast<Texture2D>(texture);
            if(tex)
            {
                find->second.texture = tex->GetTexture();
                context->PSSetShaderResources(find->second.slot, 1, &find->second.texture);

                auto find_sampler = shader_pass->ps->samplers.find(name + "_Sampler");
                if(find_sampler != shader_pass->ps->samplers.end())
                {
                    find_sampler->second.sampler = tex->GetSampler();
                    context->PSSetSamplers(find_sampler->second.slot, 1, &find_sampler->second.sampler);
                }
            }
        }
    }

	void Material::ReadyPass(int pass)
	{
		auto shader_pass = m_shader->GetPass(pass);

		for(auto &i : m_vectors)
		{
			set_constant_buffer(i.first, (void *) &i.second, sizeof(Vector4), shader_pass);
		}

		for(auto &i : m_colors)
		{
			set_constant_buffer(i.first, (void *) &i.second, sizeof(Color), shader_pass);
		}
		
		for(auto &i : m_matrices)
		{
			set_constant_buffer(i.first, (void *) &i.second, sizeof(Matrix4x4), shader_pass);
		}

		for(auto &i : m_matrix_arrays)
		{
			int size = i.second.size();
			if(size > 54)
			{
				Debug::Log("matrix array size is too big:%d", size);
			}
			else
			{
				set_constant_buffer(i.first, (void *) &i.second[0], sizeof(Matrix4x4) * size, shader_pass);
			}
		}

        for(auto &i : m_vector_arrays)
        {
            int size = i.second.size();
            if(size > 216)
            {
                Debug::Log("vector array size is too big:%d", size);
            }
            else
            {
                set_constant_buffer(i.first, (void *) &i.second[0], sizeof(Vector4) * size, shader_pass);
            }
        }

		for(auto &i: m_textures)
		{
			auto find = shader_pass->ps->textures.find(i.first);
			if(find != shader_pass->ps->textures.end())
			{
                auto tex = std::dynamic_pointer_cast<Texture2D>(i.second);
				if(tex)
				{
					find->second.texture = tex->GetTexture();

					auto find_sampler = shader_pass->ps->samplers.find(i.first + "_Sampler");
					if(find_sampler != shader_pass->ps->samplers.end())
					{
						find_sampler->second.sampler = tex->GetSampler();
					}
				}

                auto render_texture = std::dynamic_pointer_cast<RenderTexture>(i.second);
                if(render_texture)
                {
                    if(i.first == "_CameraDepthTexture")
                    {
                        find->second.texture = render_texture->GetShaderResourceViewDepth();
                    }
                    else
                    {
                        find->second.texture = render_texture->GetShaderResourceViewColor();
                    }

                    auto find_sampler = shader_pass->ps->samplers.find(i.first + "_Sampler");
                    if(find_sampler != shader_pass->ps->samplers.end())
                    {
                        find_sampler->second.sampler = render_texture->GetSamplerState();
                    }
                }
			}
		}
	}

	void Material::ApplyPass(int pass)
	{
		auto shader_pass = m_shader->GetPass(pass);
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		context->VSSetShader(shader_pass->vs->shader, NULL, 0);
		context->PSSetShader(shader_pass->ps->shader, NULL, 0);

		for(auto &i : shader_pass->vs->cbuffers)
		{
			context->VSSetConstantBuffers(i.second.slot, 1, &i.second.buffer);
		}

		for(auto &i : shader_pass->ps->cbuffers)
		{
			context->PSSetConstantBuffers(i.second.slot, 1, &i.second.buffer);
		}

		for(auto &i : shader_pass->ps->textures)
		{
            if(i.second.texture == NULL)
            {
                auto texture = Texture2D::GetDefaultTexture()->GetTexture();
                context->PSSetShaderResources(i.second.slot, 1, &texture);
            }
            else
            {
                context->PSSetShaderResources(i.second.slot, 1, &i.second.texture);
            }
		}

		for(auto &i : shader_pass->ps->samplers)
		{
            if(i.second.sampler == NULL)
            {
                auto sampler = Texture2D::GetDefaultTexture()->GetSampler();
                context->PSSetSamplers(i.second.slot, 1, &sampler);
            }
            else
            {
                context->PSSetSamplers(i.second.slot, 1, &i.second.sampler);
            }
		}
	}
}
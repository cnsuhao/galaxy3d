#include "Shader.h"
#include "GraphicsDevice.h"
#include "Application.h"
#include "Debug.h"
#include "GTFile.h"
#include "GTString.h"
#include "VertexType.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

static const int QueueNameCount = 5;
static const std::string QueueNames[QueueNameCount] =
{
	"Background",
	"Geometry",
	"AlphaTest",
	"Transparent",
	"Overlay",
};
static const int QueueValues[QueueNameCount] =
{
	1000,
	2000,
	2450,
	3000,
	4000,
};

namespace Galaxy3D
{
	std::unordered_map<std::string, std::shared_ptr<Shader>> Shader::m_shaders;
	std::unordered_map<std::string, std::string> Shader::m_paths;

	void Shader::CollectPaths(const std::string &dir)
	{
		std::string root = Application::GetDataPath();
		
		std::string find_path = root + "/" + dir;
        dirent *data;
        DIR *handle;
        if((handle = opendir(find_path.c_str())) == 0)
		{
			Debug::Log("CollectNameAndPath:file %s not found", find_path.c_str());
		}
		else
		{
            while((data = readdir(handle)) != 0)
			{
                std::string name = data->d_name;
                
                std::string full_path = root + "/" + dir + "/" + name;

                if((data->d_type & DT_DIR) != 0)
				{
					if(name != ".." && name != ".")
					{
						CollectPaths(dir + "/" + name);
					}
				}
                else if((data->d_type & DT_REG) != 0)
                {
                    struct stat st;
                    stat(full_path.c_str(), &st);

					std::string suffix(".shader");
					size_t find = name.rfind(suffix);
					if(find != std::string::npos)
					{
						std::string shader_dir = dir.substr(std::string("Assets/shader").length());
						if(!shader_dir.empty())
						{
							shader_dir = shader_dir.substr(1) + "/";
						}
						std::string shader_name = shader_dir + name.substr(0, find);
						
						m_paths[shader_name] = full_path;
					}
				}
				else
				{
                    Debug::Log("CollectNameAndPath:unknown file type %d %s", data->d_type, name.c_str());
				}
			}
                
            closedir(handle);
		}
	}

	std::shared_ptr<Shader> Shader::Find(const std::string &name)
	{
		std::shared_ptr<Shader> shader;

		auto find = m_shaders.find(name);
		if(find != m_shaders.end())
		{
			shader = find->second;
		}
		else
		{
			if(m_paths.empty())
			{
				CollectPaths("Assets/shader");
			}

			auto find_path = m_paths.find(name);
			if(find_path != m_paths.end())
			{
                std::string shader_src;
				GTFile::ReadAllText(find_path->second, shader_src);
				
				Debug::Log("create shader %s begin", name.c_str());

				auto s = std::shared_ptr<Shader>(new Shader());
				s->SetName(name);
				s->Parse(shader_src);
				s->CompileVS();
				s->CompilePS();
				s->CreateRenderStates();
				s->CreatePass();

				if(s->GetPassCount() > 0)
				{
					m_shaders[name] = s;
					shader = s;

					Debug::Log("create shader %s end", name.c_str());
				}
				else
				{
					Debug::Log("shader source error:%s", name.c_str());
				}
			}
			else
			{
				Debug::Log("can not find shader:%s", name.c_str());
			}
		}

		return shader;
	}

	Shader::Shader()
	{
	}

	Shader::~Shader()
	{
		for(auto &i : m_vertex_shaders)
		{
			i.second.Release();
		}
		m_vertex_shaders.clear();

		for(auto &i : m_pixel_shaders)
		{
			i.second.Release();
		}
		m_pixel_shaders.clear();

		for(auto &i : m_render_states)
		{
			i.second.Release();
		}
		m_render_states.clear();

		for(auto &i : m_passes)
		{
			i.Release();
		}
		m_passes.clear();
	}

	void Shader::Parse(const std::string &src)
	{
		ShaderNode *parent = 0;
		int left_count = 0;
		int last = 0;

		for(size_t i=0; i<src.length(); i++)
		{
			char c = src[i];

			if(c == '{')
			{
				ShaderNode node;

				auto type_name = GTString(src.substr(last, i-last));
				type_name = type_name.Replace("\r\n", "\n");
				type_name = type_name.Replace("\r", "\n");
				type_name = type_name.Replace("\t", "");
                type_name = type_name.Replace("    ", "");
				auto split = type_name.Split("\n", true);

				type_name = split.back().TrimSpace();
				split = type_name.Split(" ");
				node.type = split[0].str;
				if(split.size() > 1)
				{
					node.name = split[1].str;
				}
				node.start = i + 1;

				if(parent == 0)
				{
					m_shader_node = node;
					m_shader_node.parent = 0;
					parent = &m_shader_node;
				}
				else
				{
					node.parent = parent;
					parent->children.push_back(node);
					parent = &parent->children.back();
				}

				left_count++;
				last = i + 1;
				continue;
			}

			if(c == '}')
			{
				if(left_count > 0)
				{
					parent->block = src.substr(parent->start, i - parent->start);
					parent = parent->parent;

					left_count--;
					last = i + 1;

					if(left_count == 0)
					{
						break;
					}
				}
			}
		}

		for(auto &i : m_shader_node.children)
		{
			if(i.type == "Tags")
			{
				std::string queue;

				GTString block = i.block;
				block = block.Replace("\t", "");
                block = block.Replace("    ", "");
				block = block.Replace("\r", "\n");
				auto lines = block.Split("\n", true);
				for(auto j : lines)
				{
					auto splits = j.Split(" ");
					if(splits.size() == 2)
					{
						if(splits[0].str == "Queue")
						{
							queue = splits[1].str;
						}
					}
				}

				for(int j=0; j<QueueNameCount; j++)
				{
					if(queue == QueueNames[j])
					{
						m_render_queue = QueueValues[j];
						break;
					}
				}
			}
		}
	}

	void Shader::CompileVS()
	{
		for(auto &i : m_shader_node.children)
		{
			if(i.type == "GLVS")
			{
				GLuint shader = glCreateShader(GL_VERTEX_SHADER);

				const GLchar *source = i.block.c_str();
				int len = (int) i.block.size();
				glShaderSource(shader, 1, &source, &len);
				glCompileShader(shader);

				GLint log_length;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
				if(log_length > 0)
				{
					GLchar *log = (GLchar *) malloc(log_length);
					glGetShaderInfoLog(shader, log_length, &log_length, log);
					Debug::Log("Shader compile log [%s]:\n%s", m_name.c_str(), log);
					free(log);
				}

				GLint status;
				glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
				if(status == 0)
				{
					glDeleteShader(shader);
				}

				VertexShader vs;
				vs.shader = shader;

				CreateInputLayout((void *) source, len, i.block, &vs, m_name);
				CreateConstantBuffers(i.block, vs.cbuffers);

				m_vertex_shaders[i.name] = vs;
			}
		}
	}

	void Shader::CreateInputLayout(void *buffer, int size, const std::string &src, VertexShader *shader, const std::string &shader_name)
	{
		const int att_count_ui = 3;
		const std::string att_name_ui[att_count_ui] = {"Position", "Color", "Texcoord0"};
		const int att_offset_ui[att_count_ui] = {0, 3, 7};
        const int att_size_ui[att_count_ui] = {3, 4, 2};

		const int att_count_skinned = 6;
		const std::string att_name_skinned[att_count_skinned] = {"Position", "Normal", "Tangent", "BlendWeight", "BlendIndices", "Texcoord0"};
		const int att_offset_skinned[att_count_skinned] = {0, 3, 6, 10, 14, 18};
        const int att_size_skinned[att_count_skinned] = {3, 3, 4, 4, 4, 2};

		const int att_count_mesh = 5;
		const std::string att_name_mesh[att_count_mesh] = {"Position", "Normal", "Tangent", "Texcoord0", "Texcoord1"};
		const int att_offset_mesh[att_count_mesh] = {0, 3, 6, 10, 12};
        const int att_size_mesh[att_count_mesh] = {3, 3, 4, 2, 2};

		bool ui = GTString(shader_name).StartsWith("UI") || GTString(shader_name).StartsWith("Particles");
		bool skinned = GTString(shader_name).StartsWith("SkinnedMesh");

		std::vector<GTString> lines = GTString(src).Replace("\r\n", "\n").Split("\n");

		for(size_t i=0; i<lines.size(); i++)
        {
            GTString line = lines[i].Replace("\t", "").TrimSpace();

            if(line.StartsWith("attribute"))
			{
				std::vector<GTString> words = line.Split(" ");
                
                //remove empty word
                for(size_t j=0; j<words.size(); )
                {
                    if(words[j].str.empty())
                    {
                        words.erase(words.begin() + j);
                        continue;
                    }
                    
                    j++;
                }

				if(GTString(words[1]).StartsWith("vec"))
				{
					std::stringstream ss(words[1].str.substr(3));
					int value;
					ss >> value;
                    
					GTString name = words[2].Replace(";", "");
					AttributeDesc attr;
					attr.name = name.str;

					if(ui)
					{
						for(size_t j=0; j<att_count_ui; j++)
						{
							if(attr.name == att_name_ui[j])
							{
								attr.size = att_size_ui[j];
								attr.offset = att_offset_ui[j] * 4;
								shader->attributes.push_back(attr);

								break;
							}
						}
					}
					else if(skinned)
					{
						for(size_t j=0; j<att_count_skinned; j++)
						{
							if(attr.name == att_name_skinned[j])
							{
								attr.size = att_size_skinned[j];
								attr.offset = att_offset_skinned[j] * 4;
								shader->attributes.push_back(attr);
								break;
							}
						}
					}
					else
					{
						for(size_t j=0; j<att_count_mesh; j++)
						{
							if(attr.name == att_name_mesh[j])
							{
								attr.size = att_size_mesh[j];
								attr.offset = att_offset_mesh[j] * 4;
								shader->attributes.push_back(attr);
								break;
							}
						}
					}
				}
			}
		}

		if(ui)
		{
			shader->vertex_stride = sizeof(VertexUI);
		}
		else if(skinned)
		{
			shader->vertex_stride = sizeof(VertexSkinned);
		}
		else
		{
			shader->vertex_stride = sizeof(VertexMesh);
		}

		Debug::Log("attributes count %d", shader->attributes.size());
	}

	void Shader::CreateConstantBuffers(const std::string &src, std::unordered_map<std::string, ShaderConstantBuffer> &cbuffers)
	{
		std::vector<GTString> lines = GTString(src).Replace("\r\n", "\n").Split("\n");
        
		for(size_t i=0; i<lines.size(); i++)
        {
            GTString line = lines[i].Replace("\t", "").TrimSpace();

            if(line.StartsWith("uniform"))
            {
                std::vector<GTString> words = line.Split(" ");
                
                //remove empty word
                for(size_t j=0; j<words.size(); )
                {
                    if(words[j].str.empty())
                    {
                        words.erase(words.begin() + j);
                        continue;
                    }
                    
                    j++;
                }
                
                if(GTString(words[1]).StartsWith("vec"))
                {
					GTString name = words[2];
					size_t find_left;
					size_t find_right;
					int size;

					{
						std::stringstream ss(words[1].str.substr(3));
						int value;
						ss >> value;

						size = value * 4;
					}

					//array
					int array_size_value = -1;
					find_right = name.str.rfind("]");
					if(find_right != std::string::npos)
					{
						find_left = name.str.rfind("[", find_right);
						GTString array_size = name.str.substr(find_left + 1, find_right - find_left - 1);
						name = name.str.substr(0, find_left);

						std::stringstream ss(array_size.str);
						ss >> array_size_value;

						if(array_size_value > 0)
						{
							size *= array_size_value;
						}
					}
					else
					{
						name = name.Replace(";", "");
					}
                    
                    ShaderConstantBuffer cb;
                    cb.name = name.str;
                    cb.size = size;
                    
                    cbuffers[name.str] = cb;

					Debug::Log("%s %d %d", name.str.c_str(), size, array_size_value);
                }
                else if(GTString(words[1]).StartsWith("mat"))
                {
					GTString name = words[2];
					size_t find_left;
					size_t find_right;
					int size;

					{
						std::stringstream ss(words[1].str.substr(3));
						int value;
						ss >> value;

						size = value * value * 4;
					}

					//array
					int array_size_value = -1;
					find_right = name.str.rfind("]");
					if(find_right != std::string::npos)
					{
						find_left = name.str.rfind("[", find_right);
						GTString array_size = name.str.substr(find_left + 1, find_right - find_left - 1);
						name = name.str.substr(0, find_left);

						std::stringstream ss(array_size.str);
						ss >> array_size_value;

						if(array_size_value > 0)
						{
							size *= array_size_value;
						}
					}
					else
					{
						name = name.Replace(";", "");
					}
                    
                    ShaderConstantBuffer cb;
                    cb.name = name.str;
                    cb.size = size;
                    
                    cbuffers[name.str] = cb;

					Debug::Log("%s %d %d", name.str.c_str(), size, array_size_value);
                }
            }
        }
	}

	static void find_textures(const std::string &type, const std::string &src, PixelShader *shader)
	{
		std::vector<GTString> lines = GTString(src).Replace("\r\n", "\n").Split("\n");
        
		for(size_t i=0; i<lines.size(); i++)
        {
            GTString line = lines[i].Replace("\t", "").TrimSpace();
            if(line.StartsWith("uniform"))
            {
                std::vector<GTString> words = line.Split(" ");
                
                //remove empty word
                for(size_t j=0; j<words.size(); )
                {
                    if(words[j].str.empty())
                    {
                        words.erase(words.begin() + j);
                        continue;
                    }
                    
                    j++;
                }
                
                if(words[1].str == type)
                {
                    GTString name = words[2].Replace(";", "");
                    
                    ShaderTexture t;
                    t.name = name.str;
                    shader->textures[name.str] = t;

					Debug::Log("%s %s", type.c_str(), name.str.c_str());
                }
            }
        }
	}

	void Shader::CompilePS()
	{
		for(auto &i : m_shader_node.children)
		{
			if(i.type == "GLPS")
			{
				GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

				const GLchar *source = i.block.c_str();
				int len = (int) i.block.size();
				glShaderSource(shader, 1, &source, &len);
				glCompileShader(shader);

				GLint log_length;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
				if(log_length > 0)
				{
					GLchar *log = (GLchar *) malloc(log_length);
					glGetShaderInfoLog(shader, log_length, &log_length, log);
					Debug::Log("Shader compile log [%s]:\n%s", m_name.c_str(), log);
					free(log);
				}

				GLint status;
				glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
				if(status == 0)
				{
					glDeleteShader(shader);
				}

				PixelShader ps;
				ps.shader = shader;

				CreateConstantBuffers(i.block, ps.cbuffers);
				find_textures("sampler2D", i.block, &ps);
				find_textures("samplerCube", i.block, &ps);

				m_pixel_shaders[i.name] = ps;
			}
		}
	}

	void Shader::CreateRenderStates()
	{
		for(auto &i : m_shader_node.children)
		{
			if(i.type == "RenderStates")
			{
				RenderStates rs;
				rs.Parse(i.block);
				rs.Create();

				m_render_states[i.name] = rs;
			}
		}
	}

	void Shader::CreatePass()
	{
		for(auto &i : m_shader_node.children)
		{
			if(i.type == "Pass")
			{
				std::string vs;
				std::string ps;
				std::string rs;

				GTString block = i.block;
				block = block.Replace("\t", "");
                block = block.Replace("    ", "");
				block = block.Replace("\r", "\n");
				auto lines = block.Split("\n", true);
				for(auto j : lines)
				{
					auto splits = j.Split(" ");
					if(splits.size() == 2)
					{
						if(splits[0].str == "VS")
						{
							vs = splits[1].str;
						}
						else if(splits[0].str == "PS")
						{
							ps = splits[1].str;
						}
						else if(splits[0].str == "RenderStates")
						{
							rs = splits[1].str;
						}
					}
				}

				ShaderPass pass;
				pass.name = i.name;
                pass.vs = NULL;
                pass.ps = NULL;
                pass.rs = NULL;
				
				if(m_vertex_shaders.count(vs) > 0)
				{
					pass.vs = &m_vertex_shaders[vs];
				}

				if(m_pixel_shaders.count(ps) > 0)
				{
					pass.ps = &m_pixel_shaders[ps];
				}

				if(m_render_states.count(rs) > 0)
				{
					pass.rs = &m_render_states[rs];
				}

				m_passes.push_back(pass);
			}
		}

		for(auto &i : m_passes)
		{
			if(i.vs == NULL || i.ps == NULL)
			{
				continue;
			}

			GLuint program = glCreateProgram();

			glAttachShader(program, i.vs->shader);
            glAttachShader(program, i.ps->shader);

			// bind location must before link program
			int index = 0;
			for(auto &j : i.vs->attributes)
            {
				glBindAttribLocation(program, index++, j.name.c_str());
            }
			
			GLint status;
            glLinkProgram(program);
            
            GLint log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
            if(log_length > 0)
			{
                GLchar *log = (GLchar *) malloc(log_length);
                glGetProgramInfoLog(program, log_length, &log_length, log);
                Debug::Log("Program link log:\n%s", log);
                free(log);
            }

			glGetProgramiv(program, GL_LINK_STATUS, &status);
            if(status == 0)
			{
                Debug::Log("Program link error!!!");
            }

			Program p;
			p.program = program;
			p.cbuffers = i.vs->cbuffers;
			for(auto &j : i.ps->cbuffers)
            {
				if(p.cbuffers.count(j.first) == 0)
				{
					p.cbuffers[j.first] = j.second;
				}
            }

			for(auto &j : i.vs->attributes)
            {
				j.slot = glGetAttribLocation(program, j.name.c_str());

				GLint active;
				glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &active);
				GLint max;
				glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max);
				Debug::Log("attribute %s %d %d %d", j.name.c_str(), j.slot, active, max);
            }

			for(auto &j : p.cbuffers)
            {
                j.second.slot = glGetUniformLocation(program, j.first.c_str());

				Debug::Log("uniform %s %d", j.first.c_str(), j.second.slot);
            }

			for(auto &j : i.ps->textures)
            {
                j.second.slot = glGetUniformLocation(program, j.first.c_str());

				Debug::Log("uniform texture %s %d", j.first.c_str(), j.second.slot);
            }

			i.program = p;
		}

		Debug::Log("created %d passes", m_passes.size());
	}
}
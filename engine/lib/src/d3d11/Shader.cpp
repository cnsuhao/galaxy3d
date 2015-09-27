#include "Shader.h"
#include "GraphicsDevice.h"
#include "Application.h"
#include "Debug.h"
#include "GTFile.h"
#include "GTString.h"
#include "VertexType.h"

#ifdef WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#ifdef WINPC
#include <d3dcompiler.h>
#endif

static const std::string POSITION = "POSITION";
static const std::string NORMAL = "NORMAL";
static const std::string TANGENT = "TANGENT";
static const std::string COLOR = "COLOR";
static const std::string BLENDWEIGHT = "BLENDWEIGHT";
static const std::string BLENDINDICES = "BLENDINDICES";
static const std::string TEXCOORD = "TEXCOORD";
static const std::string BINORMAL = "BINORMAL";
static const std::string POSITIONT = "POSITIONT";
static const std::string POINTSIZE = "PSIZE";
	
static const int SEMANTICS_COUNT_FULL = 10;
static const std::string SEMANTICS_FULL[SEMANTICS_COUNT_FULL] =
{
	POSITION,
	NORMAL,
	TANGENT,
	COLOR,
	BLENDWEIGHT,
	BLENDINDICES,
	TEXCOORD,
	BINORMAL,
	POSITIONT,
	POINTSIZE,
};
static const DXGI_FORMAT SEMANTICS_FORMAT_FULL[SEMANTICS_COUNT_FULL] =
{
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
};
static const int SEMANTICS_SIZE_FULL[SEMANTICS_COUNT_FULL] =
{
	12, 12, 16, 16, 16, 16, 8, 16, 12, 4
};

static const int SEMANTICS_COUNT_SKINNED = 6;
static const std::string SEMANTICS_SKINNED[SEMANTICS_COUNT_SKINNED] =
{
	POSITION,
	NORMAL,
	TANGENT,
	BLENDWEIGHT,
	BLENDINDICES,
	TEXCOORD,
};
static const DXGI_FORMAT SEMANTICS_FORMAT_SKINNED[SEMANTICS_COUNT_SKINNED] =
{
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
};
static const int SEMANTICS_SIZE_SKINNED[SEMANTICS_COUNT_SKINNED] =
{
	12, 12, 16, 16, 16, 8,
};

static const int SEMANTICS_COUNT_UI = 4;
static const std::string SEMANTICS_UI[SEMANTICS_COUNT_UI] =
{
	POSITION,
	COLOR,
	TEXCOORD,
};
static const DXGI_FORMAT SEMANTICS_FORMAT_UI[SEMANTICS_COUNT_UI] =
{
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
};
static const int SEMANTICS_SIZE_UI[SEMANTICS_COUNT_UI] =
{
	12, 16, 8,
};

static const int SEMANTICS_COUNT_MESH = 5;
static const std::string SEMANTICS_MESH[SEMANTICS_COUNT_MESH] =
{
	POSITION,
	NORMAL,
	TANGENT,
	TEXCOORD,
};
static const DXGI_FORMAT SEMANTICS_FORMAT_MESH[SEMANTICS_COUNT_MESH] =
{
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
};
static const int SEMANTICS_SIZE_MESH[SEMANTICS_COUNT_MESH] =
{
	12, 12, 16, 8
};

struct TypeSize
{
	std::string name;
	int size;
};

static const int TYPE_COUNT = 18;
static const TypeSize TYPE_SIZE[TYPE_COUNT] =
{
	{"float", 4},
	{"float2", 8},
	{"float3", 12},
	{"float4", 16},
	{"float2x2", 16},
	{"float2x3", 24},
	{"float2x4", 32},
	{"float3x2", 24},
	{"float3x3", 36},
	{"float3x4", 48},
	{"float4x2", 32},
	{"float4x3", 48},
	{"float4x4", 64},
	{"matrix", 64},
	{"int", 4},
	{"int2", 8},
	{"int3", 12},
	{"int4", 16},
};

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
		
#ifdef WIN32
		std::string find_path = root + "/" + dir + "/*.*";
		_finddata_t data;
		intptr_t handle;
		if((handle = _findfirst(find_path.c_str(), &data)) == -1)
#else
		std::string find_path = root + "/" + dir;
        dirent *data;
        DIR *handle;
        if((handle = opendir(find_path.c_str())) == 0)
#endif
		{
			Debug::Log("CollectNameAndPath:file %s not found", find_path.c_str());
		}
		else
		{
#ifdef WIN32
			while(_findnext(handle, &data) == 0)
#else
            while((data = readdir(handle)) != 0)
#endif
			{
#ifdef WIN32
				std::string name = data.name;
#else
                std::string name = data->d_name;
#endif
                
                std::string full_path = root + "/" + dir + "/" + name;

#ifdef WIN32
				if((data.attrib & _A_SUBDIR) != 0)
#else
                if((data->d_type & DT_DIR) != 0)
#endif
				{
					if(name != ".." && name != ".")
					{
						CollectPaths(dir + "/" + name);
					}
				}
#ifdef WIN32
				else if((data.attrib & _A_ARCH) != 0)
                {
                    int size = data.size;
#else
                else if((data->d_type & DT_REG) != 0)
                {
                    struct stat st;
                    stat(full_path.c_str(), &st);
                    int size = (int) st.st_size;
#endif

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
#ifdef WIN32
					Debug::Log("CollectNameAndPath:unknown file type %d %s", data.attrib, name.c_str());
#else
                    Debug::Log("CollectNameAndPath:unknown file type %d %s", data->d_type, name.c_str());
#endif
				}
			}
                
#ifndef WIN32
            closedir(handle);
#endif
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
		for(auto i : m_vertex_shaders)
		{
			i.second.Release();
		}
		m_vertex_shaders.clear();

		for(auto i : m_pixel_shaders)
		{
			i.second.Release();
		}
		m_pixel_shaders.clear();

		for(auto i : m_render_states)
		{
			i.second.Release();
		}
		m_render_states.clear();
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
		auto device = GraphicsDevice::GetInstance()->GetDevice();

		for(auto &i : m_shader_node.children)
		{
			if(i.type == "HLVS")
			{
				VertexShader vs;

				HRESULT hr = S_OK;
				ID3DBlob *p_blob = 0;
				ID3DBlob *p_error = 0;

				hr = D3DCompile(i.block.c_str(), i.block.length(), 0, 0, 0, "main", "vs_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0,
					&p_blob, &p_error);

				if(FAILED(hr))
				{
					if(p_error != 0)
					{
						Debug::Log((char *) p_error->GetBufferPointer());
						p_error->Release();
					}

					return;
				}

				SAFE_RELEASE(p_error);

				hr = device->CreateVertexShader(p_blob->GetBufferPointer(), p_blob->GetBufferSize(), NULL, &vs.shader);
				if(FAILED(hr))
				{
					p_blob->Release();
					return;
				}

				CreateInputLayout(p_blob->GetBufferPointer(), p_blob->GetBufferSize(), i.block, &vs, m_name);
				CreateConstantBuffers(i.block, vs.cbuffers);

				SAFE_RELEASE(p_blob);

				m_vertex_shaders[i.name] = vs;
			}
		}
	}

	static std::vector<GTString> find_semantics(const std::string &src)
	{
		std::vector<GTString> semantics;

		std::string str_struct("struct");
		std::string VS_INPUT("VS_INPUT");

		size_t input_begin = std::string::npos;
		size_t input_end;

		//find struct VS_INPUT
		size_t pos = 0;
		size_t find_struct;
		while((find_struct = src.find(str_struct, pos)) != std::string::npos)
		{
			size_t find_input = src.find(VS_INPUT, find_struct);
			if(find_input != std::string::npos)
			{
				std::string space = src.substr(find_struct + str_struct.length(), find_input - find_struct - str_struct.length());
				
				bool all_space = true;
				for(size_t i=0; i<space.length(); i++)
				{
					if(space[i] != ' ')
					{
						all_space = false;
						break;
					}
				}

				if(all_space)
				{
					input_begin = find_input;
					break;
				}
				else
				{
					pos = find_struct + str_struct.length();
				}
			}
			else
			{
				return semantics;
			}
		}

		if(input_begin == std::string::npos)
		{
			return semantics;
		}

		//find semantics
		input_end = src.find("}", input_begin);
		if(input_end == std::string::npos)
		{
			return semantics;
		}

		pos = input_begin;
		size_t find_right;
		while((find_right = src.find(";", pos)) != std::string::npos)
		{
			if(find_right > input_end)
			{
				break;
			}
			
			size_t find_left;
			find_left = src.rfind(":", find_right);
			if(find_left == std::string::npos)
			{
				return semantics;
			}

			GTString semantic = src.substr(find_left + 1, find_right - find_left - 1);
			semantics.push_back(semantic.Replace(" ", ""));
			pos = find_right + 1;
		}

		return semantics;
	}

	static void separate_semantic(const GTString &full, GTString *name, int *index)
	{
		for(int i=(int) full.str.length()-1; i>=0; i--)
		{
			char c = full.str[i];
			if(!(c >= '0' && c <= '9'))
			{
				if(i == full.str.length()-1)
				{
					*index = 0;
					*name = full;
				}
				else
				{
					*name = full.str.substr(0, i + 1);
					std::string str_index = full.str.substr(i + 1);

					std::stringstream ss(str_index);
					ss >> *index;
				}

				return;
			}
		}
	}

	void Shader::CreateInputLayout(void *buffer, int size, const std::string &src, VertexShader *shader, const std::string &shader_name)
	{
		bool ui = GTString(shader_name).StartsWith("UI");
		bool skinned = GTString(shader_name).StartsWith("SkinnedMesh");
		
		std::vector<GTString> semantics = find_semantics(src);
		if(semantics.empty())
		{
			return;
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> &descs = shader->input_layout_descs;
		for(size_t i=0; i<semantics.size(); i++)
		{
			D3D11_INPUT_ELEMENT_DESC desc;

			GTString name("");
			int index;
			separate_semantic(semantics[i], &name, &index);

			int offset = 0;
			
			if(ui)
			{
				for(int j=0; j<SEMANTICS_COUNT_UI; j++)
				{
					if(name.str == SEMANTICS_UI[j])
					{
						desc.SemanticName = SEMANTICS_UI[j].c_str();
						desc.SemanticIndex = index;
						desc.Format = SEMANTICS_FORMAT_UI[j];
						desc.InputSlot = 0;
						desc.AlignedByteOffset = offset + index * SEMANTICS_SIZE_UI[j];
						desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
						desc.InstanceDataStepRate = 0;

						descs.push_back(desc);
						break;
					}

					offset += SEMANTICS_SIZE_UI[j];
				}
			}
			else if(skinned)
			{
				for(int j=0; j<SEMANTICS_COUNT_SKINNED; j++)
				{
					if(name.str == SEMANTICS_SKINNED[j])
					{
						desc.SemanticName = SEMANTICS_SKINNED[j].c_str();
						desc.SemanticIndex = index;
						desc.Format = SEMANTICS_FORMAT_SKINNED[j];
						desc.InputSlot = 0;
						desc.AlignedByteOffset = offset + index * SEMANTICS_SIZE_SKINNED[j];
						desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
						desc.InstanceDataStepRate = 0;

						descs.push_back(desc);
						break;
					}

					offset += SEMANTICS_SIZE_SKINNED[j];
				}
			}
			else
			{
				for(int j=0; j<SEMANTICS_COUNT_MESH; j++)
				{
					if(name.str == SEMANTICS_MESH[j])
					{
						desc.SemanticName = SEMANTICS_MESH[j].c_str();
						desc.SemanticIndex = index;
						desc.Format = SEMANTICS_FORMAT_MESH[j];
						desc.InputSlot = 0;
						desc.AlignedByteOffset = offset + index * SEMANTICS_SIZE_MESH[j];
						desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
						desc.InstanceDataStepRate = 0;

						descs.push_back(desc);
						break;
					}

					offset += SEMANTICS_SIZE_MESH[j];
				}
			}
		}

		auto device = GraphicsDevice::GetInstance()->GetDevice();
		HRESULT hr = device->CreateInputLayout(&descs[0], descs.size(), buffer, size, &shader->input_layout);

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
	}

	static ID3D11Buffer *create_constant_buffer(int size)
	{
		ID3D11Buffer *buffer;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.ByteWidth = size;

		auto device = GraphicsDevice::GetInstance()->GetDevice();
		HRESULT hr = device->CreateBuffer(&bd, NULL, &buffer);
		if(FAILED(hr))
			return 0;

		return buffer;
	}

	void Shader::CreateConstantBuffers(const std::string &src, std::unordered_map<std::string, ShaderConstantBuffer> &cbuffers)
	{
		std::string str_cbuffer = "cbuffer";
		size_t pos = 0;
		size_t find_cbuffer;
		while((find_cbuffer = src.find(str_cbuffer, pos)) != std::string::npos)
		{
			size_t find_left;
			size_t find_right;
			
			find_left = src.find("{", find_cbuffer);
			if(find_left == std::string::npos)
			{
				return;
			}

			std::string str_register = "register";
			size_t find_register = src.find(str_register, find_cbuffer);
			if(find_register == std::string::npos)
			{
				return;
			}

			if(find_register > find_left)
			{
				Debug::Log("cbuffer have no register");
				return;
			}

			find_left = src.find("(", find_register);
			find_right = src.find(")", find_register);

			GTString register_name = src.substr(find_left + 1, find_right - find_left - 1);
			register_name = register_name.Replace(" ", "");

			GTString bname("");
			int index;
			separate_semantic(register_name, &bname, &index);
			if(bname.str == "b" && index >= 0 && index < 14)
			{
				find_right = src.find(";", find_register);
				find_right = src.find_last_not_of(' ', find_right - 1);

				find_left = src.rfind(" ", find_right);
				GTString name = src.substr(find_left + 1, find_right - find_left);

				find_right = src.find_last_not_of(' ', find_left);
				find_left = src.rfind("{", find_right);
				GTString type = src.substr(find_left + 1, find_right - find_left);
				type = type.Replace("\r", "");
				type = type.Replace("\n", "");
				type = type.Replace("\t", "");
                type = type.Replace("    ", "");
				type = type.TrimSpace();
				int size = 0;

				for(int i=0; i<TYPE_COUNT; i++)
				{
					if(type.str == TYPE_SIZE[i].name)
					{
						size = TYPE_SIZE[i].size;
						break;
					}
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
				}

				if(array_size_value > 0)
				{
					size *= array_size_value;
				}

				ShaderConstantBuffer cbuffer;
				cbuffer.slot = index;
				cbuffer.size = size;
				cbuffer.buffer = create_constant_buffer(cbuffer.size);

				cbuffers[name.str] = cbuffer;

				pos = find_register + 1;
			}
			else
			{
				Debug::Log("cbuffer register error");
			}
		}
	}

	static void find_textures(const std::string &type, const std::string &src, PixelShader *shader)
	{
		size_t pos = 0;
		size_t find_Texture;
		while((find_Texture = src.find(type, pos)) != std::string::npos)
		{
			size_t find_left;
			size_t find_right;
			
			find_left = src.find(";", find_Texture);
			if(find_left == std::string::npos)
			{
				return;
			}

			std::string str_register = "register";
			size_t find_register = src.find(str_register, find_Texture);
			if(find_register == std::string::npos)
			{
				return;
			}

			if(find_register > find_left)
			{
				Debug::Log("Texture have no register");
				return;
			}

			find_left = src.find("(", find_register);
			find_right = src.find(")", find_register);

			GTString register_name = src.substr(find_left + 1, find_right - find_left - 1);
			register_name = register_name.Replace(" ", "");

			GTString tname("");
			int index;
			separate_semantic(register_name, &tname, &index);
			if(tname.str == "t" && index >= 0)
			{
				find_right = src.find(":", find_Texture);
				GTString name = src.substr(find_Texture + type.length(), find_right - find_Texture - type.length());
				name = name.Replace("\n", "");
				name = name.Replace(" ", "");

				ShaderTexture tex;
				tex.texture = NULL;
				tex.slot = index;
				shader->textures[name.str] = tex;

				pos = find_register + 1;
			}
			else
			{
				Debug::Log("Texture register error");
			}
		}
	}

	static void find_samplers(const std::string &src, PixelShader *shader)
	{
		std::string SamplerState = "SamplerState";
		size_t pos = 0;
		size_t find_SamplerState;
		while((find_SamplerState = src.find(SamplerState, pos)) != std::string::npos)
		{
			size_t find_left;
			size_t find_right;
			
			find_left = src.find(";", find_SamplerState);
			if(find_left == std::string::npos)
			{
				return;
			}

			std::string str_register = "register";
			size_t find_register = src.find(str_register, find_SamplerState);
			if(find_register == std::string::npos)
			{
				return;
			}

			if(find_register > find_left)
			{
				Debug::Log("SamplerState have no register");
				return;
			}

			find_left = src.find("(", find_register);
			find_right = src.find(")", find_register);

			GTString register_name = src.substr(find_left + 1, find_right - find_left - 1);
			register_name = register_name.Replace(" ", "");

			GTString sname("");
			int index;
			separate_semantic(register_name, &sname, &index);
			if(sname.str == "s" && index >= 0)
			{
				find_right = src.find(":", find_SamplerState);
				GTString name = src.substr(find_SamplerState + SamplerState.length(), find_right - find_SamplerState - SamplerState.length());
				name = name.Replace("\n", "");
				name = name.Replace(" ", "");

				ShaderSampler sam;
				sam.sampler = NULL;
				sam.slot = index;
				shader->samplers[name.str] = sam;

				pos = find_register + 1;
			}
			else
			{
				Debug::Log("SamplerState register error");
			}
		}
	}

	void Shader::CompilePS()
	{
		auto device = GraphicsDevice::GetInstance()->GetDevice();

		for(auto &i : m_shader_node.children)
		{
			if(i.type == "HLPS")
			{
				PixelShader ps;

				HRESULT hr = S_OK;
				ID3DBlob *p_blob = 0;
				ID3DBlob *p_error = 0;

				hr = D3DCompile(i.block.c_str(), i.block.length(), 0, 0, 0, "main", "ps_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0,
					&p_blob, &p_error);

				if(FAILED(hr))
				{
					if(p_error != 0)
					{
						Debug::Log((char *) p_error->GetBufferPointer());
						p_error->Release();
					}

					return;
				}

				SAFE_RELEASE(p_error);

				hr = device->CreatePixelShader(p_blob->GetBufferPointer(), p_blob->GetBufferSize(), NULL, &ps.shader);
				if(FAILED(hr))
				{
					p_blob->Release();
					return;
				}

				CreateConstantBuffers(i.block, ps.cbuffers);
				find_textures("Texture2D", i.block, &ps);
				find_textures("TextureCube", i.block, &ps);
				find_samplers(i.block, &ps);

				SAFE_RELEASE(p_blob);

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
	}
}
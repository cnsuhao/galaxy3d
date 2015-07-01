#ifndef __Shader_h__
#define __Shader_h__

#include "Object.h"
#include "ShaderNode.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "RenderStates.h"
#include "ShaderPass.h"
#include <unordered_map>

namespace Galaxy3D
{
	class Shader : public Object
	{
	public:
		static std::shared_ptr<Shader> Find(const std::string &name);
		~Shader();
		int GetRenderQueue() const {return m_render_queue;}
		int GetPassCount() const {return m_passes.size();}
		ShaderPass *GetPass(int pass) {return &m_passes[pass];}

	private:
		static std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
		static std::unordered_map<std::string, std::string> m_paths;
		ShaderNode m_shader_node;
		std::unordered_map<std::string, VertexShader> m_vertex_shaders;
		std::unordered_map<std::string, PixelShader> m_pixel_shaders;
		std::unordered_map<std::string, RenderStates> m_render_states;
		std::vector<ShaderPass> m_passes;
		int m_render_queue;

		static void CollectPaths(const std::string &dir);
		static void CreateInputLayout(void *buffer, int size, const std::string &src, VertexShader *shader, const std::string &shader_name);
		static void CreateConstantBuffers(const std::string &src, std::unordered_map<std::string, ShaderConstantBuffer> &cbuffers);
		Shader();
		void Parse(const std::string &src);
		void CompileVS();
		void CompilePS();
		void CreateRenderStates();
		void CreatePass();
	};
}

#endif
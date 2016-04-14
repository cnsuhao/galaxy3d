#ifndef __VertexShader_h__
#define __VertexShader_h__

#include "ShaderConstantBuffer.h"
#include "GraphicsDevice.h"
#include <vector>
#include <unordered_map>

namespace Galaxy3D
{
	struct AttributeDesc
    {
        std::string name;
        int slot;
        int size;
		int offset;
    };

	struct VertexShader
	{
		GLuint shader;
		std::vector<AttributeDesc> attributes;
		int vertex_stride;
		std::unordered_map<std::string, ShaderConstantBuffer> cbuffers;

		VertexShader():shader(0),vertex_stride(-1) {}
		void Release()
		{
			if(shader != 0)
			{
				glDeleteShader(shader);
				shader = 0;
			}

			for(auto i : cbuffers)
			{
				i.second.Release();
			}
			cbuffers.clear();
		}
	};
}

#endif
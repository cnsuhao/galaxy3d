#ifndef __VertexShader_h__
#define __VertexShader_h__

#include "ShaderConstantBuffer.h"
#include "GraphicsDevice.h"
#include <d3d11.h>
#include <vector>
#include <unordered_map>

namespace Galaxy3D
{
	struct VertexShader
	{
		ID3D11VertexShader *shader;
		ID3D11InputLayout *input_layout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_descs;
		int vertex_stride;
		std::unordered_map<std::string, ShaderConstantBuffer> cbuffers;

		VertexShader():shader(nullptr),input_layout(nullptr),vertex_stride(-1) {}
		void Release()
		{
			SAFE_RELEASE(shader);
			SAFE_RELEASE(input_layout);
			for(auto i : cbuffers)
			{
				i.second.Release();
			}
			cbuffers.clear();
		}
	};
}

#endif
#ifndef __PixelShader_h__
#define __PixelShader_h__

#include "ShaderConstantBuffer.h"
#include "ShaderTexture.h"
#include "ShaderSampler.h"
#include "GraphicsDevice.h"
#include <d3d11.h>
#include <unordered_map>

namespace Galaxy3D
{
	struct PixelShader
	{
		ID3D11PixelShader *shader;
		std::unordered_map<std::string, ShaderConstantBuffer> cbuffers;
		std::unordered_map<std::string, ShaderTexture> textures;
		std::unordered_map<std::string, ShaderSampler> samplers;

		PixelShader():shader(nullptr) {}
		void Release()
		{
			SAFE_RELEASE(shader);

			for(auto i : cbuffers)
			{
				i.second.Release();
			}
			cbuffers.clear();
		}
	};
}

#endif
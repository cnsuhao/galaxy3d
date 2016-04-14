#ifndef __PixelShader_h__
#define __PixelShader_h__

#include "ShaderConstantBuffer.h"
#include "ShaderTexture.h"
#include "GraphicsDevice.h"
#include <unordered_map>

namespace Galaxy3D
{
	struct PixelShader
	{
		GLuint shader;
		std::unordered_map<std::string, ShaderConstantBuffer> cbuffers;
		std::unordered_map<std::string, ShaderTexture> textures;

		PixelShader():shader(0) {}
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
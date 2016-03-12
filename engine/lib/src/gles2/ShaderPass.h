#ifndef __ShaderPass_h__
#define __ShaderPass_h__

#include "VertexShader.h"
#include "PixelShader.h"
#include "RenderStates.h"
#include <string>

namespace Galaxy3D
{
	struct Program
	{
		GLuint program;
		std::unordered_map<std::string, ShaderConstantBuffer> cbuffers;

		Program():program(0){}
	};

	struct ShaderPass
	{
		std::string name;
		VertexShader *vs;
		PixelShader *ps;
		RenderStates *rs;
		Program program;

		void Release()
		{
			if(program.program != 0)
			{
				glDeleteProgram(program.program);
				program.program = 0;
			}
		}
	};
}

#endif
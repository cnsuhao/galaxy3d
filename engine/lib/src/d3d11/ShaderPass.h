#ifndef __ShaderPass_h__
#define __ShaderPass_h__

#include "VertexShader.h"
#include "PixelShader.h"
#include "RenderStates.h"
#include <string>

namespace Galaxy3D
{
	struct ShaderPass
	{
		std::string name;
		VertexShader *vs;
		PixelShader *ps;
		RenderStates *rs;
	};
}

#endif
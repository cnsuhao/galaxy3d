#ifndef __ShaderTexture_h__
#define __ShaderTexture_h__

#include "GraphicsDevice.h"

namespace Galaxy3D
{
	struct ShaderTexture
	{
		std::string name;
		GLuint texture;
		GLuint cubemap;
		int slot;

		ShaderTexture():texture(0),cubemap(0),slot(-1) {}
	};
}

#endif
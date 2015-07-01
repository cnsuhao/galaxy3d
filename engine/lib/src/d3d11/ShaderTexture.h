#ifndef __ShaderTexture_h__
#define __ShaderTexture_h__

#include "GraphicsDevice.h"
#include <d3d11.h>

namespace Galaxy3D
{
	struct ShaderTexture
	{
		ID3D11ShaderResourceView *texture;
		int slot;

		ShaderTexture():texture(nullptr),slot(-1) {}
	};
}

#endif
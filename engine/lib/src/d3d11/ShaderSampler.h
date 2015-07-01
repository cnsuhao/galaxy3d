#ifndef __ShaderSampler_h__
#define __ShaderSampler_h__

#include "GraphicsDevice.h"
#include <d3d11.h>

namespace Galaxy3D
{
	struct ShaderSampler
	{
		ID3D11SamplerState *sampler;
		int slot;

		ShaderSampler():sampler(nullptr),slot(-1) {}
	};
}

#endif
#ifndef __ShaderConstantBuffer_h__
#define __ShaderConstantBuffer_h__

#include "GraphicsDevice.h"
#include <d3d11.h>

namespace Galaxy3D
{
	struct ShaderConstantBuffer
	{
		ID3D11Buffer *buffer;
		int slot;
		int size;

		ShaderConstantBuffer():buffer(NULL),slot(-1),size(-1) {}
		void Release()
		{
			SAFE_RELEASE(buffer);
		}
	};
}

#endif
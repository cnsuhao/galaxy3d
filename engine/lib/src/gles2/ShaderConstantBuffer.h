#ifndef __ShaderConstantBuffer_h__
#define __ShaderConstantBuffer_h__

#include "GraphicsDevice.h"

namespace Galaxy3D
{
	struct ShaderConstantBuffer
	{
		std::string name;
		int slot;
		int size;

		ShaderConstantBuffer():slot(-1),size(-1) {}
		void Release()
		{
		}
	};
}

#endif
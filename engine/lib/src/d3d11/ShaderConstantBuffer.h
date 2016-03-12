#ifndef __ShaderConstantBuffer_h__
#define __ShaderConstantBuffer_h__

#include "GraphicsDevice.h"

namespace Galaxy3D
{
	struct ShaderConstantBuffer
	{
		ID3D11Buffer *buffer;
		int slot;
		int size;
        void *cpu_buffer;

		ShaderConstantBuffer():buffer(NULL),slot(-1),size(-1),cpu_buffer(NULL) {}
		void Release()
		{
			SAFE_RELEASE(buffer);
            if(cpu_buffer != NULL)
            {
                free(cpu_buffer);
            }
		}
        void FillCPUBuffer(void *data, int size)
        {
            if(cpu_buffer == NULL)
            {
                cpu_buffer = malloc(this->size);
            }

            memcpy(cpu_buffer, data, size);
        }
	};
}

#endif
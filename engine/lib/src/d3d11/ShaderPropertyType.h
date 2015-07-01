#ifndef __ShaderPropertyType_h__
#define __ShaderPropertyType_h__

namespace Galaxy3D
{
	struct ShaderPropertyType
	{
		enum Enum
		{
			Color = 0,
			Vector = 1,
			Float = 2,
			Range = 3,
			TexEnv = 4,
		};
	};
}

#endif
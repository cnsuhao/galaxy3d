#ifndef __CameraClearFlags_h__
#define __CameraClearFlags_h__

namespace Galaxy3D
{
	struct CameraClearFlags
	{
		enum Enum
		{
			Skybox = 1,
			SolidColor = 2,
			Depth = 3,
			Nothing = 4,
		};
	};
}


#endif
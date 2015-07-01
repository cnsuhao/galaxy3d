#ifndef __FilterMode_h__
#define __FilterMode_h__

namespace Galaxy3D
{
	struct FilterMode
	{
		enum Enum
		{
			Point = 0,
			Bilinear = 1,
			Trilinear = 2,
		};
	};
}

#endif
#ifndef __Layer_h__
#define __Layer_h__

namespace Galaxy3D
{
	struct Layer
	{
		enum
		{
			Default = 0,
			TransparentFX = 1,
			IgnoreRaycast = 2,
            Highlighting = 3,
			Water = 4,
			UI = 5,
            Scene = 6,
            Character = 7,
		};
	};
}

#endif
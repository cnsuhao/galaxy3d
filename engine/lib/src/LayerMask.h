#ifndef __LayerMask_h__
#define __LayerMask_h__

namespace Galaxy3D
{
	class LayerMask
	{
	public:
		static int GetMask(int layer) {return 1 << layer;}
        static int All() {return -1;}
        static int None() {return 0;}
	};
}

#endif
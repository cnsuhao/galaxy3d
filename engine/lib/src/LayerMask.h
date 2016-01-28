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
        static bool IsCulled(int layer, int mask) {return ((1 << layer) & mask) == 0;}
	};
}

#endif
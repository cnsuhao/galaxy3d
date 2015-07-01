#ifndef __LayerMask_h__
#define __LayerMask_h__

namespace Galaxy3D
{
	class LayerMask
	{
	public:
		static int GetMask(int layer) {return 1 << layer;}
	};
}

#endif
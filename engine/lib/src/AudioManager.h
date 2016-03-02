#ifndef __AudioManager_h__
#define __AudioManager_h__

#include "Vector3.h"

namespace Galaxy3D
{
	class AudioListener;

	class AudioManager
	{
	public:
		static void Init();
		static void Done();
		static void SetVolume(float volume);
		static void SetListener(AudioListener *listener);
	};
}

#endif
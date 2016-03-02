#ifndef __AudioListener_h__
#define __AudioListener_h__

#include "Component.h"

namespace Galaxy3D
{
	class AudioListener : public Component
	{
	public:
		static void Pause();
		static void Resume();
		static void SetVolume(float volume);
		static float GetVolume() {return m_volume;}

	protected:
		virtual void Start();

	private:
		static bool m_paused;
		static float m_volume;
	};
}

#endif
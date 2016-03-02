#include "AudioListener.h"
#include "AudioManager.h"

namespace Galaxy3D
{
	bool AudioListener::m_paused = false;
	float AudioListener::m_volume = 1.0f;

	void AudioListener::SetVolume(float volume)
	{
		m_volume = volume;

		AudioManager::SetVolume(volume);
	}

	void AudioListener::Pause()
	{
		m_paused = true;
	}

	void AudioListener::Resume()
	{
		m_paused = false;
	}

	void AudioListener::Start()
	{
		AudioManager::SetListener(this);
	}
}
#include "AudioSource.h"
#include "AudioManager.h"
#include "Mathf.h"

namespace Galaxy3D
{
	AudioSource::~AudioSource()
	{
		AudioManager::DeleteSource(this);
	}

	void AudioSource::Awake()
	{
		m_source = AudioManager::CreateSource(this);
	}

	void AudioSource::OnTranformChanged()
	{
		AudioManager::SetSourcePosition(this);
	}

	void AudioSource::SetLoop(bool loop)
	{
		if(m_loop != loop)
		{
			m_loop = loop;

			if(m_source != NULL)
			{
				AudioManager::SetSourceLoop(this);
			}
		}
	}

	void AudioSource::SetClip(const std::shared_ptr<AudioClip> &clip)
	{
		if(m_clip != clip)
		{
			m_clip = clip;

			if(m_source != NULL)
			{
				AudioManager::SetSourceBuffer(this);
			}
		}
	}

	void AudioSource::SetVolume(float volume)
	{
		if(!Mathf::FloatEqual(m_volume, volume))
		{
			m_volume = volume;

			if(m_source != NULL)
			{
				AudioManager::SetSourceVolume(this);
			}
		}
	}

	void AudioSource::SetTime(float time)
	{
		AudioManager::SetSourceOffset(this, time);
	}

	float AudioSource::GetTime()
	{
		return AudioManager::GetSourceOffset(this);
	}

	void AudioSource::Play()
	{
		AudioManager::PlaySource(this);
	}
}
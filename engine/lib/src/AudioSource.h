#ifndef __AudioSource_h__
#define __AudioSource_h__

#include "Component.h"

namespace Galaxy3D
{
	class AudioClip;

	class AudioSource : public Component
	{
	public:
		AudioSource():
			m_source(NULL),
			m_loop(false),
			m_volume(1.0f),
			m_mp3_buffer(NULL)
		{}
		virtual ~AudioSource();
		void SetClip(const std::shared_ptr<AudioClip> &clip);
		std::shared_ptr<AudioClip> GetClip() const {return m_clip;}
		void *GetSource() const {return m_source;}
		void SetLoop(bool loop);
		bool IsLoop() const {return m_loop;}
		void SetVolume(float volume);
		float GetVolume() const {return m_volume;}
		void SetTime(float time);
		float GetTime();
		void Play();
		void PlayMp3File(const std::string &file);

	protected:
		virtual void Awake();
		virtual void Update();
		virtual void OnTranformChanged();

	private:
		std::shared_ptr<AudioClip> m_clip;
		void *m_source;
		bool m_loop;
		float m_volume;
		void *m_mp3_buffer;
	};
}

#endif
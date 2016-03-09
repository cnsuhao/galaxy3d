#ifndef __AudioClip_h__
#define __AudioClip_h__

#include "Object.h"

namespace Galaxy3D
{
	class AudioClip : public Object
	{
	public:
		static std::shared_ptr<AudioClip> LoadFromFile(const std::string &file);
		virtual ~AudioClip();
		int GetChannels() const {return m_channels;}
		int GetBits() const {return m_bits;}
		int GetBufferSize() const {return m_size;}
		int GetFrequency() const {return m_frequency;}
		void *GetBuffer() const {return m_buffer;}

	private:
		int m_channels;
		int m_frequency;
		int m_samples;
		float m_length;
		int m_size;
		int m_bits;
		void *m_buffer;

		AudioClip():
			m_channels(0),
			m_frequency(0),
			m_samples(0),
			m_length(0),
			m_size(0),
			m_bits(0),
			m_buffer(NULL)
		{}
	};
}

#endif
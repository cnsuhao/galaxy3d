#include "AudioSource.h"
#include "AudioManager.h"
#include "Mathf.h"
#include "Debug.h"
#include "GTFile.h"
#include "GTTime.h"
#include "mad.h"
#include <thread>
#include <chrono>
#include <mutex>

namespace Galaxy3D
{
	struct Mp3Buffer
	{
		static const int PCM_BUFFER_SIZE = 8096;
		const unsigned char *data_mp3;
		int data_mp3_size;
		int channel;
		int frequency;
		int bits;
		char data_pcm[PCM_BUFFER_SIZE];
		int data_pcm_pos;
		bool loop;
		bool done;
		bool play;
		AudioSource *source;
		std::thread *thread;
		std::mutex *mutex;
		bool exit;

		void WriteSample(unsigned char *bytes, int channel)
		{
			if(data_pcm_pos + 2 <= PCM_BUFFER_SIZE)
			{
				data_pcm[data_pcm_pos] = bytes[0];
				data_pcm[data_pcm_pos + 1] = bytes[1];

				data_pcm_pos += 2;
			}

			if(data_pcm_pos == PCM_BUFFER_SIZE)
			{
				const int QUEUED_BUFFER_MAX = 100;
				const int SLEEP_TIME_MS = 100;

				mutex->lock();
				int queued = AudioManager::GetSourceBufferQueued(source);
				mutex->unlock();

				while(queued >= QUEUED_BUFFER_MAX && !exit)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));

					mutex->lock();
					queued = AudioManager::GetSourceBufferQueued(source);
					mutex->unlock();
				}

				// queue buffer to al
				mutex->lock();
				auto buffer = AudioManager::CreateBuffer(channel, frequency, bits, data_pcm, PCM_BUFFER_SIZE);
				AudioManager::SetSourceQueueBuffer(source, buffer);
				if(!play)
				{
					play = true;
					AudioManager::PlaySource(source);
				}
				mutex->unlock();

				data_pcm_pos = 0;
			}
		}
	};

	static mad_flow mp3_header(void *data, const mad_header *header)
	{
		Mp3Buffer *buffer = (Mp3Buffer *) data;

		buffer->frequency = header->samplerate;
		buffer->bits = 16;

		return MAD_FLOW_CONTINUE;
	}

	static int mp3_scale_sample(mad_fixed_t sample)
	{
		/* round */
		sample += (1L << (MAD_F_FRACBITS - 16));

		/* clip */
		if(sample >= MAD_F_ONE)
			sample = MAD_F_ONE - 1;
		else if(sample < -MAD_F_ONE)
			sample = -MAD_F_ONE;

		/* quantize */
		return sample >> (MAD_F_FRACBITS + 1 - 16);
	}

	static mad_flow mp3_error(
		void *data,
		mad_stream *stream,
		mad_frame *frame)
	{
		Mp3Buffer *buffer = (Mp3Buffer *) data;

		Debug::Log("mp3 decoding error 0x%04x (%s) at byte offset %u\n",
			stream->error, mad_stream_errorstr(stream),
			stream->this_frame - buffer->data_mp3);

		/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow mp3_input(void *data, mad_stream *stream)
	{
		Mp3Buffer *buffer = (Mp3Buffer *) data;

		if (buffer->data_mp3_size <= 0)
			return MAD_FLOW_STOP;

		mad_stream_buffer(stream, buffer->data_mp3, buffer->data_mp3_size);

		if(!buffer->loop)
		{
			buffer->data_mp3_size = 0;
		}

		if(buffer->exit)
		{
			return MAD_FLOW_STOP;
		}

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow mp3_output(
		void *data,
		mad_header const *header,
		mad_pcm *pcm)
	{
		Mp3Buffer *buffer = (Mp3Buffer *) data;

		unsigned int nchannels, nsamples;
		mad_fixed_t const *left_ch, *right_ch;

		/* pcm->samplerate contains the sampling frequency */

		nchannels = pcm->channels;
		nsamples = pcm->length;
		left_ch = pcm->samples[0];
		right_ch = pcm->samples[1];

		while(nsamples--)
		{
			int sample;

			/* output sample(s) in 16-bit signed little-endian PCM */
			unsigned char bytes[2];

			sample = mp3_scale_sample(*left_ch++);
			bytes[0] = (sample >> 0) & 0xff;
			bytes[1] = (sample >> 8) & 0xff;

			buffer->WriteSample(bytes, nchannels);

			if(nchannels == 2)
			{
				sample = mp3_scale_sample(*right_ch++);
				bytes[0] = (sample >> 0) & 0xff;
				bytes[1] = (sample >> 8) & 0xff;

				buffer->WriteSample(bytes, nchannels);
			}
		}

		if(buffer->exit)
		{
			return MAD_FLOW_STOP;
		}

		return MAD_FLOW_CONTINUE;
	}

	static void mp3_decode(Mp3Buffer *buffer, const std::string &file)
	{
		int size;
		auto bytes = GTFile::ReadAllBytes(file, &size);
		if(bytes != NULL)
		{
			buffer->data_mp3 = (const unsigned char *) bytes;
			buffer->data_mp3_size = size;
			buffer->data_pcm_pos = 0;

			mad_decoder decoder;
			mad_decoder_init(&decoder, buffer,
			   mp3_input, mp3_header, NULL /* filter */, mp3_output,
			   mp3_error, NULL /* message */);

			mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
			mad_decoder_finish(&decoder);

			free(bytes);
		}

		buffer->done = true;
	}

	void AudioSource::PlayMp3File(const std::string &file)
	{
		if(m_mp3_buffer == NULL)
		{
			auto mp3_buffer = new Mp3Buffer();
			m_mp3_buffer = mp3_buffer;

			memset(mp3_buffer, 0, sizeof(Mp3Buffer));
			mp3_buffer->loop = m_loop;
			mp3_buffer->source = this;
			mp3_buffer->mutex = new std::mutex();
			mp3_buffer->thread = new std::thread(mp3_decode, (Mp3Buffer *) mp3_buffer, file);
		}
	}

	AudioSource::~AudioSource()
	{
		if(m_mp3_buffer != NULL)
		{
			auto mp3_buffer = (Mp3Buffer *) m_mp3_buffer;
			// wait for thread exit
			mp3_buffer->exit = true;
			mp3_buffer->thread->join();
			delete mp3_buffer->thread;
			delete mp3_buffer->mutex;
			delete mp3_buffer;
			m_mp3_buffer = NULL;

			// delete all buffers queued in source
			AudioManager::DeleteSourceBufferQueue(this);
		}

		AudioManager::DeleteSource(this);
	}

	void AudioSource::Awake()
	{
		m_source = AudioManager::CreateSource(this);
	}

	void AudioSource::Update()
	{
		if(m_mp3_buffer != NULL)
		{
			auto mp3_buffer = (Mp3Buffer *) m_mp3_buffer;
			mp3_buffer->mutex->lock();
			AudioManager::ProcessSourceBufferQueue(this);
			mp3_buffer->mutex->unlock();
		}
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
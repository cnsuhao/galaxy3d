#include "AudioClip.h"
#include "GTFile.h"
#include "AudioManager.h"

namespace Galaxy3D
{
	static bool is_wave(char *bytes)
	{
		if(memcmp(bytes, "RIFF" , 4) == 0)
		{
			if(memcmp(&bytes[8], "WAVE" , 4) == 0)
			{
				if(memcmp(&bytes[12], "fmt " , 4) == 0)
				{
					return true;
				}
			}
		}

		return false;
	}

	std::shared_ptr<AudioClip> AudioClip::LoadFromFile(const std::string &file)
	{
		std::shared_ptr<AudioClip> clip;

		if(GTFile::Exist(file))
		{
			int size;
			char *bytes = (char *) GTFile::ReadAllBytes(file, &size);

			if(is_wave(bytes))
			{
				auto c = std::shared_ptr<AudioClip>(new AudioClip());

				char *p = bytes;

				int chunk_data_pos = 8;
				int chunk_size;
				bool data_found = false;
				short block_align;

				p += 4;
				BUFFER_READ(chunk_size, p, 4);
				chunk_size = 4;
				chunk_data_pos = p - bytes;

				while(!data_found && p - bytes < size)
				{
					// got to next chunk
					int cur_pos = p - bytes;
					int offset = chunk_size - (cur_pos - chunk_data_pos);
					p += offset;

					char chunk_id[4];
					BUFFER_READ(chunk_id[0], p, 4);
					BUFFER_READ(chunk_size, p, 4);
					chunk_data_pos = p - bytes;

					if(memcmp(chunk_id, "fmt " , 4) == 0)
					{
						short fmt;
						BUFFER_READ(fmt, p, 2);
						if(fmt != 1)
						{
							break;
						}

						short channels;
						BUFFER_READ(channels, p, 2);
						if(channels > 2)
						{
							break;
						}
						c->m_channels = channels;
						
						int sample_rate;
						BUFFER_READ(sample_rate, p, 4);
						c->m_frequency = sample_rate;

						p += 4;

						BUFFER_READ(block_align, p, 2);

						short bits;
						BUFFER_READ(bits, p, 2);
						c->m_bits = bits;
					}
					else if(memcmp(chunk_id, "data" , 4) == 0)
					{
						data_found = true;
						c->m_size = chunk_size;
					}
				}

				c->m_samples = c->m_size / block_align;

				if(data_found)
				{
					c->m_buffer = AudioManager::CreateClipBuffer(c.get(), p);

					clip = c;
				}
			}

			free(bytes);
		}

		return clip;
	}

	AudioClip::~AudioClip()
	{
		if(m_buffer != NULL)
		{
			AudioManager::DeleteClipBuffer(this);
			m_buffer = NULL;
		}
	}
}
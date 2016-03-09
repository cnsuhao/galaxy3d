#include "AudioManager.h"
#include "AudioListener.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "Transform.h"
#include "Debug.h"
#include "AL/al.h"
#include "AL/alc.h"
#include <vector>
#include <mutex>

#define OAL_DEVICE_ID 0

namespace Galaxy3D
{
	typedef std::lock_guard<std::mutex> MutexLock;

	static ALCdevice *g_device;
	static ALCcontext *g_context;
	static std::mutex g_context_mutex;

	static std::vector<std::string> get_devices()
	{
		std::vector<std::string> devices;

		const ALchar* devicesStr = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		const ALCchar *device = devicesStr, *next = devicesStr + 1;
        size_t len = 0;

        while (device && *device != '\0' && next && *next != '\0') {
			devices.push_back(device);

			len = strlen(device);
			device += (len + 1);
			next += (len + 2);
        }

		return devices;
	}

	bool AudioManager::IsInitComplete()
	{
		MutexLock lock(g_context_mutex);
		return g_context != NULL;
	}

	void AudioManager::Init()
	{
		MutexLock lock(g_context_mutex);

		auto devices = get_devices();

		auto result = alcOpenDeviceAsync(
			devices[OAL_DEVICE_ID].c_str(),
			[=](ALCdevice *device)
			{
				MutexLock lock(g_context_mutex);

				if(device != NULL)
				{
					g_device = device;

					auto context = alcCreateContext(device, NULL);
					if(context != NULL)
					{
						g_context = context;

						auto result = alcMakeContextCurrent(context);
						if(result == ALC_FALSE)
						{
							Debug::Log("alcMakeContextCurrent failed");
						}
					}
					else
					{
						Debug::Log("alcCreateContext failed");
					}
				}
				else
				{
					Debug::Log("alcOpenDeviceAsync get NULL device");
				}
			}
		);

		if(result == ALC_FALSE)
		{
			Debug::Log("alcOpenDeviceAsync failed");
		}
	}

	void AudioManager::Done()
	{
		if(g_context)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(g_context);
			g_context = NULL;
		}

		if(g_device)
		{
			alcCloseDevice(g_device);
			g_device = NULL;
		}
	}

	void AudioManager::SetVolume(float volume)
	{
		alListenerf(AL_GAIN, volume);
	}

	void AudioManager::SetListener(AudioListener *listener)
	{
		auto pos = listener->GetTransform()->GetPosition();
		auto forward = listener->GetTransform()->GetForward();
		auto up = listener->GetTransform()->GetUp();

		float orientation[] =
		{
			forward.x, forward.y, forward.z,
			up.x, up.y, up.z,
		};
		float velocity[] = {0, 0, 0};

		alListenerfv(AL_POSITION, (float *) &pos);
		alListenerfv(AL_ORIENTATION, orientation);
		alListenerfv(AL_VELOCITY, velocity);
	}

	void *AudioManager::CreateClipBuffer(AudioClip *clip, void *data)
	{
		ALenum format = 0;

		int bits = clip->GetBits();
		switch(clip->GetChannels())
		{
		case 1:
			if(bits == 8)
				format = AL_FORMAT_MONO8;
			else
				format = AL_FORMAT_MONO16;
			break;
		case 2:
			if(bits == 8)
				format = AL_FORMAT_STEREO8;
			else
				format = AL_FORMAT_STEREO16;
			break;
		}

		ALuint buffer = 0;
		alGenBuffers(1, &buffer);
		if(buffer > 0)
		{
			alBufferData(buffer, format, data, clip->GetBufferSize(), clip->GetFrequency());
		}
		else
		{
			Debug::Log("alGenBuffers failed");
		}

		return (void *) buffer;
	}

	void AudioManager::DeleteClipBuffer(AudioClip *clip)
	{
		ALuint buffer = (ALuint) clip->GetBuffer();
		alDeleteBuffers(1, &buffer);
	}

	void *AudioManager::CreateSource(AudioSource *source)
	{
		ALuint src = 0;
		alGenSources(1, &src);

		SetSourcePosition(source);
		SetSourceLoop(source);
		SetSourceVolume(source);
		if(source->GetClip())
		{
			SetSourceBuffer(source);
		}

		return (void *) src;
	}

	void AudioManager::DeleteSource(AudioSource *source)
	{
		ALuint src = (ALuint) source->GetSource();
		alDeleteSources(1, &src);
	}

	void AudioManager::SetSourcePosition(AudioSource *source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcefv(src, AL_POSITION, (ALfloat *) &source->GetTransform()->GetPosition());
	}

	void AudioManager::SetSourceLoop(AudioSource *source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcei(src, AL_LOOPING, source->IsLoop());
	}

	void AudioManager::SetSourceBuffer(AudioSource *source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcei(src, AL_BUFFER, (ALuint) source->GetClip()->GetBuffer());
	}

	void AudioManager::SetSourceVolume(AudioSource *source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcef(src, AL_GAIN, source->GetVolume());
	}

	void AudioManager::SetSourceOffset(AudioSource *source, float time)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcef(src, AL_SEC_OFFSET, time);
	}

	float AudioManager::GetSourceOffset(AudioSource *source)
	{
		float time;
		ALuint src = (ALuint) source->GetSource();
		alGetSourcef(src, AL_SEC_OFFSET, &time);

		return time;
	}

	void AudioManager::PlaySource(AudioSource *source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcePlay(src);
	}
}
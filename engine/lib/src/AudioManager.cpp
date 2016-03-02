#include "AudioManager.h"
#include "AudioListener.h"
#include "Transform.h"
#include "Debug.h"
#include "AL/al.h"
#include "AL/alc.h"
#include <vector>

#define OAL_DEVICE_ID 0

namespace Galaxy3D
{
	static ALCdevice *g_device;
	static ALCcontext *g_context;

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

	void AudioManager::Init()
	{
		auto devices = get_devices();

		auto result = alcOpenDeviceAsync(
			devices[OAL_DEVICE_ID].c_str(),
			[=](ALCdevice *device)->void
			{
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

						alcMakeContextCurrent(NULL);
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
}
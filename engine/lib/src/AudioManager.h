#ifndef __AudioManager_h__
#define __AudioManager_h__

#include "Vector3.h"

namespace Galaxy3D
{
	class AudioListener;
	class AudioClip;
	class AudioSource;

	class AudioManager
	{
	public:
		static void Init();
		static bool IsInitComplete();
		static void Done();
		static void OnPause();
		static void OnResume();
		static void SetVolume(float volume);
		static void SetListener(AudioListener *listener);
		static void *CreateClipBuffer(AudioClip *clip, void *data);
		static void DeleteClipBuffer(AudioClip *clip);
		static void *CreateBuffer(int channel, int frequency, int bits, void *data, int size);
		static void *CreateSource(AudioSource *source);
		static void DeleteSource(AudioSource *source);
		static void SetSourcePosition(AudioSource *source);
		static void SetSourceLoop(AudioSource *source);
		static void SetSourceBuffer(AudioSource *source);
		static void SetSourceQueueBuffer(AudioSource *source, void *buffer);
		static void ProcessSourceBufferQueue(AudioSource *source);
		static void DeleteSourceBufferQueue(AudioSource *source);
		static int GetSourceBufferQueued(AudioSource *source);
		static void SetSourceVolume(AudioSource *source);
		static void SetSourceOffset(AudioSource *source, float time);
		static float GetSourceOffset(AudioSource *source);
		static void PlaySource(AudioSource *source);
	};
}

#endif
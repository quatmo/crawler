/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#ifndef SOLOUD_INTERNAL_H
#define SOLOUD_INTERNAL_H

#include "soloud.h"

namespace SoLoud
{
	// SDL back-end initialization call
	result sdl_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// SDL "non-dynamic" back-end initialization call
	result sdlstatic_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// SDL2 "non-dynamic" back-end initialization call
	result sdl2static_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// OpenAL back-end initialization call
	result openal_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// PortAudio back-end initialization call
	result portaudio_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// WinMM back-end initialization call
	result winmm_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 4096);

	// XAudio2 back-end initialization call
	result xaudio2_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// WASAPI back-end initialization call
	result wasapi_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 4096);

	// OSS back-end initialization call
	result oss_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// ALSA back-end initialization call
	result alsa_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// null driver back-end initialization call
	result null_init(SoLoud::Soloud *aSoloud, unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aSamplerate = 44100, unsigned int aBuffer = 2048);

	// Deinterlace samples in a buffer. From 12121212 to 11112222
	void deinterlace_samples(const float *aSourceBuffer, float *aDestBuffer, unsigned int aSamples, unsigned int aChannels);

	// Interlace samples in a buffer. From 11112222 to 12121212
	void interlace_samples(const float *aSourceBuffer, float *aDestBuffer, unsigned int aSamples, unsigned int aChannels);
};

#define FOR_ALL_VOICES_PRE \
		handle *h_ = NULL; \
		handle th_[2] = { aVoiceHandle, 0 }; \
		if (mLockMutexFunc) mLockMutexFunc(mMutex); \
		h_ = voiceGroupHandleToArray(aVoiceHandle); \
		if (h_ == NULL) h_ = th_; \
		while (*h_) \
		{ \
			int ch = getVoiceFromHandle(*h_); \
			if (ch != -1)  \
			{

#define FOR_ALL_VOICES_POST \
			} \
			h_++; \
		} \
		if (mUnlockMutexFunc) mUnlockMutexFunc(mMutex);

#define FOR_ALL_VOICES_PRE_3D \
		handle *h_ = NULL; \
		handle th_[2] = { aVoiceHandle, 0 }; \
		h_ = voiceGroupHandleToArray(aVoiceHandle); \
		if (h_ == NULL) h_ = th_; \
				while (*h_) \
						{ \
			int ch = (*h_ & 0xfff) - 1; \
			if (ch != -1 && m3dData[ch].mHandle == *h_)  \
						{

#define FOR_ALL_VOICES_POST_3D \
						} \
			h_++; \
						} 

#endif
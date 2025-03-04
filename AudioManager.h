#pragma once
#include <xaudio2.h>
#include <x3daudio.h>
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include "Input.h"

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif
/*
   Much of this code was adapted from YouTube user Cakez's XAudio2 tutorial
   (https://www.youtube.com/watch?v=38A6WmBvxHM), so credit to them for this
*/


// Constant literals
constexpr WORD BITSPERSSAMPLE = 16;                                                      // 16 bits per sample.
constexpr DWORD SAMPLESPERSEC = 44100;                                                   // 44,100 samples per second.
constexpr double CYCLESPERSEC = 220.0;                                                   // 220 cycles per second (frequency of the audible tone).
constexpr double VOLUME = 0.5;                                                           // 50% volume. Potentially better practice to set this value in the game loop?
constexpr WORD AUDIOBUFFERSIZEINCYCLES = 10;                                             // 10 cycles per audio buffer.
constexpr double PI = 3.14159265358979323846;
constexpr WORD NUM_CHANNELS = 2;														 // 2 audio channel to test.

// Calculated constants
constexpr DWORD SAMPLESPERCYCLE = (DWORD)(SAMPLESPERSEC / CYCLESPERSEC);                 // 200 samples per cycle.
constexpr DWORD AUDIOBUFFERSIZEINSAMPLES = SAMPLESPERCYCLE * AUDIOBUFFERSIZEINCYCLES;    // 2,000 samples per buffer.
constexpr UINT32 AUDIOBUFFERSIZEINBYTES = AUDIOBUFFERSIZEINSAMPLES * BITSPERSSAMPLE / 8; // 4,000 bytes per buffer.

// Other sound-related constants
constexpr WORD MAX_CONCURRENT_SOUNDS = 16;												 // 16 sounds can play at once.
constexpr WORD MAX_CACHED_SOUNDS = 1;													 // 16 sounds can be cached at once.
static constexpr int SOUNDS_BUFFER_SIZE = 1024000000;                                    // 128 MB sound buffer size. This is needed because sounds (music especially) can have very large file sizes.
constexpr WORD MAX_SOUND_PATH_LENGTH = 256;												 // Maximum sound path size of 256 characters.

// Sound struct
struct Sound
{
private:
	std::string fileName;
	XAUDIO2_BUFFER buffer;
	int numOfPlayingVoices;
	bool inCache;

public:
	/// <summary>
	/// Returns the file name of the sound.
	/// </summary>
	/// <returns>The file name of the sound.</returns>
	std::string GetFileName()
	{
		return fileName;
	}
	/// <summary>
	/// Returns a pointer to the sound's XAUDIO2_BUFFER.
	/// </summary>
	/// <returns>A pointer to the sound's XAUDIO2_BUFFER.</returns>
	XAUDIO2_BUFFER* GetBuffer()
	{
		return &buffer;
	}

	/// <summary>
	/// Default constructor, don't use. When creating a sound, use the other constructor.
	/// </summary>
	Sound()
	{
		fileName = "";
		buffer = { 0 };
		numOfPlayingVoices = 0;
		inCache = false;
	}
	/// <summary>
	/// Creates a Sound object, storing the corresponding file path
	/// and XAUDIO2_BUFFER struct.
	/// </summary>
	/// <param name="fileName">The sound's file path.</param>
	/// <param name="buffer">The sound's XAUDIO2_BUFFER struct.</param>
	Sound(std::string fileName, XAUDIO2_BUFFER buffer)
	{
		this->fileName = fileName;
		this->buffer = buffer;
		this->buffer.pContext = this;
		numOfPlayingVoices = 0;
		inCache = false;
	}
	/// <summary>
	/// Deletes the sound. The FreeSoundData() struct must be called
	/// before deleting the sound object.
	/// </summary>
	~Sound()
	{

	}
	/// <summary>
	/// Frees up the memory used by the sound buffer. Must be called
	/// before deleting the sound struct.
	/// </summary>
	void FreeSoundData()
	{
		delete buffer.pAudioData;
		buffer.pAudioData = nullptr;
		buffer.pContext = nullptr;
	}
	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="other">The other Sound object.</param>
	Sound(const Sound& other)
	{
		FreeSoundData();
		fileName = other.fileName;
		numOfPlayingVoices = other.numOfPlayingVoices;
		buffer = other.buffer;
		buffer.pContext = this;
		// By default, assume that this sound isn't in the cache
		inCache = false;
	}

	friend class XAudioVoice;
	friend class AudioManager;
};

// XAudioVoice struct
// TODO: Whenever some kind of GameObject is made, allow them to use their own
// XAudioVoice for playing positional audio.
struct XAudioVoice : IXAudio2VoiceCallback
{
public:
	bool playing = false;
	bool isPositional = false;
	IXAudio2SourceVoice* voice;

	X3DAUDIO_EMITTER* GetEmitter()
	{
		return &emitter;
	}

	void Init3DEmitter()
	{
		emitter.ChannelCount = 1; // If problems show up, up the channel count to 2
		emitter.CurveDistanceScaler = emitter.DopplerScaler = 1.0f;
	}

	void UpdateEmitter(X3DAUDIO_VECTOR front, X3DAUDIO_VECTOR up, X3DAUDIO_VECTOR pos,
		X3DAUDIO_VECTOR vel)
	{
		emitter.OrientFront = front;
		emitter.OrientTop = up;
		emitter.Position = pos;
		emitter.Velocity = vel;
	}

	void OnStreamEnd() noexcept
	{
		voice->Stop();
		playing = false;
	}

	// When the buffer starts, increment the amount of voices playing this sound.
	void OnBufferStart(void* pBufferContext) noexcept 
	{
		((Sound*)pBufferContext)->numOfPlayingVoices++;
	}

	// When the buffer ends, increment the amount of voices playing this sound.
	// Also, if the sound isn't in the cache, delete it to prevent memory leaks.
	void OnBufferEnd(void* pBufferContext) noexcept
	{
		((Sound*)pBufferContext)->numOfPlayingVoices--;
		//std::cout << ((Sound*)pBufferContext)->numOfPlayingVoices << std::endl;
		if (!((Sound*)pBufferContext)->inCache)
		{
			std::cout << ((Sound*)pBufferContext)->fileName << " isn't in the cache. Deleting." << std::endl;
			((Sound*)pBufferContext)->FreeSoundData();
			delete ((Sound*)pBufferContext);
		}
	}

	// Methods that need to be defined but not scripted, could do cool stuff with them later
	void OnVoiceProcessingPassEnd() noexcept {}
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {}
	void OnLoopEnd(void* pBufferContext) noexcept {}
	void OnVoiceError(void* pBufferContext, HRESULT error) noexcept {}
private:
	X3DAUDIO_EMITTER emitter = {};
};

class AudioManager
{
public:
	AudioManager();
	~AudioManager();
	void playSound(const char filePath[MAX_SOUND_PATH_LENGTH]);
	Sound* create_sound(const char filePath[MAX_SOUND_PATH_LENGTH]);
	void cache_sound(Sound* sound);
	void update_audio(float dt);
	void UpdateListener(X3DAUDIO_VECTOR front, X3DAUDIO_VECTOR up, X3DAUDIO_VECTOR pos,
		X3DAUDIO_VECTOR vel);

private:
	static XAudioVoice voiceArr[MAX_CONCURRENT_SOUNDS];
	Sound* cachedSounds[MAX_CACHED_SOUNDS];
	IXAudio2* xAudio2;
	X3DAUDIO_HANDLE X3DInstance;
	X3DAUDIO_DSP_SETTINGS DSPSettings;
	X3DAUDIO_LISTENER mainListener;
	bool init();
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
};


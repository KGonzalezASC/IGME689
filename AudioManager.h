#pragma once
#include <xaudio2.h>
#include <memory>
#include <vector>
#include <iostream>
#include <string>

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
constexpr WORD NUM_CHANNELS = 2;														 // 1 audio channel to test.

// Calculated constants
constexpr DWORD SAMPLESPERCYCLE = (DWORD)(SAMPLESPERSEC / CYCLESPERSEC);                 // 200 samples per cycle.
constexpr DWORD AUDIOBUFFERSIZEINSAMPLES = SAMPLESPERCYCLE * AUDIOBUFFERSIZEINCYCLES;    // 2,000 samples per buffer.
constexpr UINT32 AUDIOBUFFERSIZEINBYTES = AUDIOBUFFERSIZEINSAMPLES * BITSPERSSAMPLE / 8; // 4,000 bytes per buffer.

// Other sound-related constants
constexpr WORD MAX_CONCURRENT_SOUNDS = 16;												 // 16 sounds can play at once.
static constexpr int SOUNDS_BUFFER_SIZE = 1024000000;                                    // 128 MB sound buffer size. This is needed because sounds (music especially) can have very large file sizes.
constexpr WORD MAX_SOUND_PATH_LENGTH = 256;												 // Maximum sound path size of 256 characters.

// Sound struct
struct Sound
{
private:
	std::string fileName;
	XAUDIO2_BUFFER buffer;

public:
	std::string GetFileName()
	{
		return fileName;
	}
	XAUDIO2_BUFFER* GetBuffer()
	{
		return &buffer;
	}
	Sound(std::string fileName, XAUDIO2_BUFFER buffer)
	{
		this->fileName = fileName;
		this->buffer = buffer;
		this->buffer.pContext = this;
	}
	~Sound()
	{
		delete buffer.pAudioData;
		buffer.pAudioData = nullptr;
		buffer.pContext = nullptr;
	}
};

// XAudioVoice struct
struct XAudioVoice : IXAudio2VoiceCallback
{
public:
	bool playing = false;
	IXAudio2SourceVoice* voice;

	void OnStreamEnd() noexcept
	{
		voice->Stop();
		playing = false;
	}

	// TEMP: Delete the audio data from the buffer. pBufferContext is set to the buffer's pAudioData var in playSound(), so this
	// should ideally always free up that memory. Will be replaced with audio pooling and caching later down the line,
	// I just don't like seeing really big memory leaks
	void OnBufferEnd(void* pBufferContext) noexcept
	{
		delete (Sound*)pBufferContext;
	}

	// Methods that need to be defined but not scripted, could do cool stuff with them later
	void OnBufferStart(void* pBufferContext) noexcept {};
	void OnVoiceProcessingPassEnd() noexcept {}
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {}
	void OnLoopEnd(void* pBufferContext) noexcept {}
	void OnVoiceError(void* pBufferContext, HRESULT error) noexcept {}
};

class AudioManager
{
public:
	AudioManager();
	~AudioManager();
	void playSound(const char filePath[MAX_SOUND_PATH_LENGTH]);
	Sound* create_sound(const char filePath[MAX_SOUND_PATH_LENGTH]);
	void update_audio(float dt);

private:
	static XAudioVoice voiceArr[MAX_CONCURRENT_SOUNDS];
	IXAudio2* xAudio2;
	bool init();
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
};


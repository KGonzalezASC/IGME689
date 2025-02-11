#pragma once
#include <xaudio2.h>
#include <memory>
#include <vector>
#include <iostream>
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
constexpr WORD MAX_CONCURRENT_SOUNDS = 32;												 // 16 sounds can play at once.
constexpr WORD MAX_SOUND_CACHE_SIZE = 16;
static constexpr int SOUNDS_BUFFER_SIZE = 1024000000;                                    // 128 MB sound buffer size. This is needed because sounds (music especially) can have very large file sizes.
constexpr WORD MAX_SOUND_PATH_LENGTH = 256;												 // Maximum sound path size of 256 characters.

// Sound struct
struct Sound
{
private:
	std::string filePath = "";
	XAUDIO2_BUFFER audioBuffer = { 0 };
	static HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;

		while (hr == S_OK)
		{
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());


			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}

			dwOffset += sizeof(DWORD) * 2;

			if (dwChunkType == fourcc)
			{
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}

			dwOffset += dwChunkDataSize;

			if (bytesRead >= dwRIFFDataSize) return S_FALSE;
		}

		return S_OK;
	}

	static HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

public:
	int numOfPlayingVoices = -1;
	bool inCache = false;
	std::string GetFilePath()
	{
		return filePath;
	}

	XAUDIO2_BUFFER* GetBuffer()
	{
		return &audioBuffer;
	}
	
	Sound()
	{
		// Needed a default constructor because otherwise Visual Studio would yell at me :(
	}

	Sound(std::string fileName)
	{
		// Declare WAVEFORMATEX and XAUDIO2_BUFFER structs
		WAVEFORMATEXTENSIBLE wfx = { 0 };
		audioBuffer = { 0 };

		std::cout << filePath << std::endl;
		// Load the file
		HANDLE hFile = CreateFileA(
			fileName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		DWORD dwChunkSize;
		DWORD dwChunkPosition;

		//check the file type, should be fourccWAVE or 'XWMA'
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
		{
			std::cout << "File is not fourccWAVE" << std::endl;
			return;
		}

		// Locate the FMT chunk, and copy its contents into a WAVEFORMATEX structure 
		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

		// Locate the data chunk, and read its contents into a buffer
		FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		BYTE* pDataBuffer = new BYTE[dwChunkSize];
		ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

		// Populate the XAUDIO2_BUFFER structure
		audioBuffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		audioBuffer.pAudioData = pDataBuffer;  //buffer containing audio data
		audioBuffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
		audioBuffer.pContext = this;

		inCache = false;
		numOfPlayingVoices = 0;
		filePath = fileName;
	}

	~Sound()
	{
		//delete[] &audioBuffer;
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


	void OnBufferStart(void* pBufferContext) noexcept
	{
		((Sound*)pBufferContext)->numOfPlayingVoices++;
	};

	// Decrease the number of number of voices playing this sound. If the sound
	// isn't in any cache, delete it.
	void OnBufferEnd(void* pBufferContext) noexcept
	{
		Sound* sound = ((Sound*)pBufferContext);
		sound->numOfPlayingVoices--;
		if (!sound->inCache)
			delete sound;
	}

	// Methods that need to be defined but not scripted, could do cool stuff with them later
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
	void playSound(std::string filePath);
	void update_audio(float dt);
	bool AddSoundToCache(Sound* sound);

private:
	static XAudioVoice voiceArr[MAX_CONCURRENT_SOUNDS];
	IXAudio2* xAudio2;
	static Sound cachedSounds[MAX_SOUND_CACHE_SIZE];
	bool init();
	bool PlayBuffer(XAUDIO2_BUFFER* buffer);
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
};
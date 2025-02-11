#include "AudioManager.h"
using namespace Input;

XAudioVoice AudioManager::voiceArr[MAX_CONCURRENT_SOUNDS];
Sound AudioManager::cachedSounds[MAX_SOUND_CACHE_SIZE];

AudioManager::AudioManager()
{
	bool initSuccess = init();
	if (!initSuccess)
	{
		// TODO: Throw error if initialization failed
	}
}

AudioManager::~AudioManager()
{
	// Stop every currently playing sound
	for (int idx = 0; idx < MAX_CONCURRENT_SOUNDS; idx++)
	{
		XAudioVoice* voice = &voiceArr[idx];
		// If an active voice is found, stop its playback early
		if (voice->playing)
		{
			voice->voice->Stop();
			voice->voice->FlushSourceBuffers();
		}
	}

	// Delete every sound in the cache
	for (int idx = 0; idx < MAX_SOUND_CACHE_SIZE; idx++)
	{
		if (cachedSounds[idx].GetFilePath() != "")
			delete &cachedSounds[idx];
	}

	// Release the reference to the XAudio2 engine
	if (xAudio2)
	{
		xAudio2->Release();
		xAudio2 = nullptr;
	}

	// Uninitialize COM
	CoUninitialize();
}

void AudioManager::playSound(std::string filePath)
{
	// Check the cache array for a sound with the same file name.
	for (int idx = 0; idx < MAX_SOUND_CACHE_SIZE; idx++)
	{
		// If one is found, play that sound on an open voice and return early
		if (filePath == cachedSounds[idx].GetFilePath())
		{
			std::cout << "Played " << cachedSounds[idx].GetFilePath() << " from cache idx " << idx << std::endl;
			PlayBuffer(cachedSounds[idx].GetBuffer());
			return;
		}
	}

	// If there are no sounds in the cache with that file name, create a new Sound struct
	std::cout << filePath << std::endl;
	Sound* newSound = new Sound(filePath);

	// If there aren't any open voices, delete the sound and return early
	if (!PlayBuffer(newSound->GetBuffer()))
	{
		// Don't forget to delete the unused memory :)
		delete newSound;
		return;
	}

	// Add the new sound struct to the cache
	AddSoundToCache(newSound);

	// TODO: Allow a sound to be played if there aren't any spots in the cache (have it get
	// deleted after the sound stops playing)
	// TODO: Allow a sound to be cached even if it can't be played if there's an empty spot 
	// in the cache
}

bool AudioManager::init()
{
	// Initialize COM
	HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		std::cout << "Failed at initialize com" << std::endl;
		return false;
	}

	// Create an instance of the XAudio2 engine
	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		std::cout << "Failed at XAudio2 engine creation" << std::endl;
		return false;
	}

	// Create a mastering voice
	IXAudio2MasteringVoice* masteringVoice = nullptr;
	hr = xAudio2->CreateMasteringVoice(&masteringVoice);
	if (FAILED(hr))
	{
		std::cout << "Failed at mastering voice creation" << std::endl;
		return false;
	}

	// Define a format
	WAVEFORMATEX wave = {};
	wave.wFormatTag = WAVE_FORMAT_PCM;
	wave.nChannels = NUM_CHANNELS;
	wave.nSamplesPerSec = SAMPLESPERSEC;
	wave.wBitsPerSample = BITSPERSSAMPLE;
	wave.nBlockAlign = NUM_CHANNELS * BITSPERSSAMPLE / 8;
	wave.nAvgBytesPerSec = SAMPLESPERSEC * wave.nBlockAlign;

	// Initialize the array of voices
	for (int idx = 0; idx < MAX_CONCURRENT_SOUNDS; idx++)
	{
		XAudioVoice* voice = &voiceArr[idx];
		hr = xAudio2->CreateSourceVoice(&voice->voice, &wave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voice);
		voice->voice->SetVolume(VOLUME);
		if (FAILED(hr))
		{
			std::cout << "Failed at voice creation" << std::endl;
			return false;
		}
	}

	// Everything has been set up successfully, return true
	return true;
}

void AudioManager::update_audio(float dt)
{
	// Until update_audio has anything better to do, have it play funny sounds when different keys are pressed
	if (Input::KeyPress('1')) 
		playSound("Sounds/vine-boom.wav");
	if (Input::KeyPress('2'))
		playSound("Sounds/amongus-roundstart.wav");
	if (Input::KeyPress('3'))
		playSound("Sounds/metal-pipe.wav");
	if (Input::KeyPress('4'))
		playSound("Sounds/oof.wav");
	if (Input::KeyPress('5'))
		playSound("Sounds/ping.wav");
	if (Input::KeyPress('6'))
		playSound("Sounds/yippee-tbh.wav");
	if (Input::KeyPress('7'))
		playSound("Sounds/yahoo.wav");
	if (Input::KeyPress('8'))
		playSound("Sounds/baka-mitai.wav");
}

HRESULT AudioManager::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
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

HRESULT AudioManager::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

bool AudioManager::PlayBuffer(XAUDIO2_BUFFER* buffer)
{
	// Check if there are any inactive voices (has to go after file loading since that takes a while)
	IXAudio2SourceVoice* chosenVoice = nullptr;
	for (int idx = 0; idx < MAX_CONCURRENT_SOUNDS; idx++)
	{
		XAudioVoice* voice = &voiceArr[idx];
		// If an inactive voice is found, submit the source buffer to it, play the sound, and break the loop early
		if (!voice->playing)
		{
			std::cout << idx << std::endl;
			chosenVoice = voice->voice;
			voice->playing = true;
			break;
		}
	}

	// If all voices are playing, return false
	if (nullptr == chosenVoice)
	{
		// Print corresponding message and return
		printf("All voices are playing sounds. Skipping audio playback\n");
		return false;
	}

	// Otherwise, play the sound and return true
	chosenVoice->SubmitSourceBuffer(buffer);
	chosenVoice->Start(0);
	return true;
}

bool AudioManager::AddSoundToCache(Sound* sound)
{
	// Search the cache array for a non-playing sound or an empty spot
	int nonPlayingSoundIdx = -1;
	for (int idx = 0; idx < MAX_SOUND_CACHE_SIZE; idx++)
	{
		// If a sound isn't being played on any voices, store its index to the corresponding value
		if (cachedSounds[idx].numOfPlayingVoices == 0 && nonPlayingSoundIdx < 0)
			nonPlayingSoundIdx = idx;
		// If an empty spot in the array is found, store the corresponding data there and return true
		else if (cachedSounds[idx].GetBuffer() == nullptr)
		{
			cachedSounds[idx] = *sound;
			sound->inCache = true;
			std::cout << "Added " << sound->GetFilePath() << " to idx " << idx << std::endl;
			return true;
		}
	}

	// If a non-playing sound is found, delete it, replace it with the provided sound, and return true
	if (nonPlayingSoundIdx >= 0)
	{
		delete& cachedSounds[nonPlayingSoundIdx];
		cachedSounds[nonPlayingSoundIdx] = *sound;
		sound->inCache = true;
		std::cout << "Overwrote the sound at idx " << nonPlayingSoundIdx << " with " << sound->GetFilePath() << std::endl;
		return true;
	}
	// Otherwise, the cache is full. Print a corresponding message and return false
	else
	{
		printf("The cache is full. The sound will not be cached.");
		return false;
	}
}
#include "AudioManager.h"

XAudioVoice AudioManager::voiceArr[MAX_CONCURRENT_SOUNDS];

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

	// Release the reference to the XAudio2 engine
	if (xAudio2)
	{
		xAudio2->Release();
		xAudio2 = nullptr;
	}

	CoUninitialize();
}

void AudioManager::playSound(const char filePath[MAX_SOUND_PATH_LENGTH])
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

	// If there aren't any open voices, return early
	if (nullptr == chosenVoice)
	{
		// Don't forget to delete the unused memory :)
		//delete[] pDataBuffer;

		// Print corresponding message and return
		printf("All voices are playing sounds. Skipping audio playback\n");
		return;
	}

	// Create a new Sound struct
	Sound* newSound = create_sound(filePath);

	// Play the sound effect
	chosenVoice->SubmitSourceBuffer(newSound->GetBuffer());
	chosenVoice->Start(0);
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
	//if (Input::KeyPress('1')) 
	//	playSound("Sounds/vine-boom.wav");
	//if (Input::KeyPress('2'))
	//	playSound("Sounds/amongus-roundstart.wav");
	//if (Input::KeyPress('3'))
	//	playSound("Sounds/metal-pipe.wav");
	//if (Input::KeyPress('4'))
	//	playSound("Sounds/oof.wav");
	//if (Input::KeyPress('5'))
	//	playSound("Sounds/ping.wav");
	//if (Input::KeyPress('6'))
	//	playSound("Sounds/yippee-tbh.wav");
	//if (Input::KeyPress('7'))
	//	playSound("Sounds/yahoo.wav");
	//if (Input::KeyPress('8'))
	//	playSound("Sounds/baka-mitai.wav");
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
			//Current error: dwChunkType isn't a fourccRIFF
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

Sound* AudioManager::create_sound(const char filePath[MAX_SOUND_PATH_LENGTH])
{
	// Declare WAVEFORMATEX and XAUDIO2_BUFFER structs
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	HANDLE hFile = CreateFileA(
		filePath,
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
		return nullptr;
	}

	// Locate the FMT chunk, and copy its contents into a WAVEFORMATEX structure 
	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

	// Locate the data chunk, and read its contents into a buffer
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	// Populate the XAUDIO2_BUFFER structure
	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	// Create the sound struct and return it
	Sound* newSound = new Sound(filePath, buffer);
	return newSound;
}
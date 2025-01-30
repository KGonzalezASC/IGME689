#include "AudioManager.h"

// Commented out because I was getting ahead of myself
//AudioManager& AudioManager::Get()
//{
//	static AudioManager instance;
//	return instance;
//}
//
//AudioManager::AudioManager()
//{
//	bool initSuccess = init();
//	if (!initSuccess)
//	{
//		// TODO: Throw error if initialization failed
//	}
//}
//
//AudioManager::~AudioManager()
//{
//	if (xAudio2)
//	{
//		xAudio2->Release();
//		xAudio2 = nullptr;
//	}
//
//	CoUninitialize();
//}
//
//void AudioManager::playSound(Sound sound)
//{
//	// Check if there are any inactive voices
//	for (int idx = 2; idx < MAX_CONCURRENT_SOUNDS; idx++) // First two voices are reserved for music
//	{
//		XAudioVoice* voice = &voiceArr[idx];
//		// If an inactive voice is found, play the sound on it
//		if (!voice->playing)
//		{
//			// TODO: Store the most recently-played sounds to a cache
//			HANDLE hFile = CreateFile(
//				sound.GetFileName(),
//				GENERIC_READ,
//				FILE_SHARE_READ,
//				NULL,
//				OPEN_EXISTING,
//				0,
//				NULL);
//
//			DWORD dwChunkSize;
//			DWORD dwChunkPosition;
//			//check the file type, should be fourccWAVE or 'XWMA'
//			FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
//			DWORD filetype;
//			ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
//			if (filetype != fourccWAVE)
//				return S_FALSE;
//		}
//	}
//}
//
//bool AudioManager::init()
//{
//	// Initialize COM
//	HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
//	if (FAILED(hr)) return false;
//
//	// Create an instance of the XAudio2 engine
//	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
//	if (FAILED(hr)) return false;
//
//	// Create a mastering voice
//	IXAudio2MasteringVoice* masteringVoice = nullptr;
//	hr = xAudio2->CreateMasteringVoice(&masteringVoice);
//	if (FAILED(hr)) return false;
//
//	// Define a format
//	WAVEFORMATEX wave = {};
//	wave.wFormatTag = WAVE_FORMAT_PCM;
//	wave.nChannels = NUM_CHANNELS;
//	wave.nSamplesPerSec = SAMPLESPERSEC;
//	wave.wBitsPerSample = BITSPERSSAMPLE;
//	wave.nBlockAlign = NUM_CHANNELS * BITSPERSSAMPLE / 8;
//	wave.nAvgBytesPerSec = SAMPLESPERSEC * wave.nBlockAlign;
//
//	// Initialize the array of voices
//	for (int idx = 0; idx < MAX_CONCURRENT_SOUNDS; idx++)
//	{
//		XAudioVoice* voice = &voiceArr[idx];
//		hr = xAudio2->CreateSourceVoice(&voice->voice, &wave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voice);
//		voice->voice->SetVolume(VOLUME);
//		if (FAILED(hr)) return false;
//	}
//
//	// Everything has been set up successfully, return true
//	return true;
//}
//
//void AudioManager::update_audio(float dt)
//{
//	// Check every actively playing sound
//	//for (int soundIdx = 0; soundIdx < soundState.playingSounds.size(); soundIdx++)
//	//{
//	//	Sound& sound = soundState.playingSounds[soundIdx];
//	//	// Check if the sound actually has data
//	//}
//}
//
//HRESULT AudioManager::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
//{
//	HRESULT hr = S_OK;
//	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
//		return HRESULT_FROM_WIN32(GetLastError());
//	DWORD dwRead;
//	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
//		hr = HRESULT_FROM_WIN32(GetLastError());
//	return hr;
//}
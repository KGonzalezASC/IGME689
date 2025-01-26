#include "AudioManager.h"

AudioManager::AudioManager()
{
	bool initSuccess = init();
	if (!initSuccess)
	{
		// TODO: Throw error if initialization failed
	}
}

bool AudioManager::init()
{
	// Initialize COM
	HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) return false;

	// Create an instance of the XAudio2 engine
	IXAudio2* xAudio2 = nullptr;
	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr)) return false;

	// Create a mastering voice
	IXAudio2MasteringVoice* masteringVoice = nullptr;
	hr = xAudio2->CreateMasteringVoice(&masteringVoice);
	if (FAILED(hr)) return false;

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
		if (FAILED(hr)) return false;
	}

	// Everything has been set up successfully, return true
	return true;
}

void AudioManager::update_audio(float dt)
{
	// Check every actively playing sound
	for (int soundIdx = 0; soundIdx < soundState.playingSounds.size(); soundIdx++)
	{
		Sound& sound = soundState.playingSounds[soundIdx];
		// Check if the sound actually has data
		
		// TO-DO: Tutorial has sound options, implement similar functionality

	}
}
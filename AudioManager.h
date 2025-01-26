#pragma once
#include <xaudio2.h>
#include <memory>
#include <vector>
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
constexpr WORD NUM_CHANNELS = 1;														 // 1 audio channel to test.

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
	char file[MAX_SOUND_PATH_LENGTH];
	int size;
	char* data;
};

// XAudioVoice struct
struct XAudioVoice : IXAudio2VoiceCallback
{
	IXAudio2SourceVoice* voice;
	bool playing;

	void OnStreamEnd() noexcept
	{
		voice->Stop();
		playing = false;
	}

	void OnBufferStart(void* pBufferContext) noexcept
	{
		playing = true;
	}

	// Methods that need to be defined but not scripted, could do cool stuff with them later
	void OnVoiceProcessingPassEnd() noexcept {}
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {}
	void OnBufferEnd(void *pBufferContext) noexcept {}
	void OnLoopEnd(void* pBufferContext) noexcept {}
	void OnVoiceError(void* pBufferContext, HRESULT error) noexcept {}
};

// SoundState struct
struct SoundState
{
	// Buffer containing all sounds
	int bytesUsed;
	char* allocatedSoundsBuffer;
	// TODO: Tutorial uses a bump allocater, need to create one or find an equivalent system

	// The sounds currently allocated in memory.
	std::vector<Sound> allocatedSounds = {};

	// The sounds that are currently playing.
	std::vector<Sound> playingSounds = {};

	// TODO: Replace vectors with arrays to limit the amount of played sounds to 16
	// and remove the need for a constructor.
	SoundState()
	{
		bytesUsed = 0;
		allocatedSoundsBuffer = {};
		allocatedSounds.reserve(16);
		playingSounds.reserve(16);
	}
};

class AudioManager
{
public:
	AudioManager();
	~AudioManager();
	void playSound(char* soundName);
	void update_audio(float dt);

private:
	static XAudioVoice voiceArr[MAX_CONCURRENT_SOUNDS];
	static SoundState soundState;
	bool init();
};


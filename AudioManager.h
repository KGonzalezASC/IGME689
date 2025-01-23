#pragma once
#include <xaudio2.h>
#include <memory>
/*
   Much of this code was adapted from YouTube user Cakez's XAudio2 tutorial
   (https://www.youtube.com/watch?v=38A6WmBvxHM), so credit to them for this
*/


// Constant literals
constexpr WORD   BITSPERSSAMPLE = 16;                                                    // 16 bits per sample.
constexpr DWORD  SAMPLESPERSEC = 44100;                                                  // 44,100 samples per second.
constexpr double CYCLESPERSEC = 220.0;                                                   // 220 cycles per second (frequency of the audible tone).
constexpr double VOLUME = 0.5;                                                           // 50% volume.
constexpr WORD   AUDIOBUFFERSIZEINCYCLES = 10;                                           // 10 cycles per audio buffer.
constexpr double PI = 3.14159265358979323846;
constexpr WORD   NUM_CHANNELS = 1;														 // 1 audio channel to test.
constexpr WORD   MAXCONCURRENTSOUNDS = 16;												 // 16 sounds can play at once.

// Calculated constants
constexpr DWORD  SAMPLESPERCYCLE = (DWORD)(SAMPLESPERSEC / CYCLESPERSEC);                // 200 samples per cycle.
constexpr DWORD  AUDIOBUFFERSIZEINSAMPLES = SAMPLESPERCYCLE * AUDIOBUFFERSIZEINCYCLES;   // 2,000 samples per buffer.
constexpr UINT32 AUDIOBUFFERSIZEINBYTES = AUDIOBUFFERSIZEINSAMPLES * BITSPERSSAMPLE / 8; // 4,000 bytes per buffer.

// Sound struct
struct Sound
{
	char file[256];
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
};

class AudioManager
{
public:
	AudioManager();
	~AudioManager();
	void play_sound(char* soundName);
	void update_audio();

private:
	static XAudioVoice voiceArr[MAXCONCURRENTSOUNDS];
	bool init();
};


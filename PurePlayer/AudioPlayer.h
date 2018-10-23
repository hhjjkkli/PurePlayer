#pragma once
#include "BasePlayer.h"
#include "AudioDecoder.h"
#include "ManagerPlayer.h"

class AudioPlayer:public BasePlayer
{
public:
	AudioPlayer(ManagerPlayer * mPlayer);
	~AudioPlayer();
	/*void play_current_frame(void * managerPlayer);
	void play_current_frame() {};*/
	void play_current_frame();
	bool get_aud_buffer(uint8_t * outputBuffer);
	static int play_audio_sdl();
	static void prepare();

	AudioDecoder audioDecoder;
	static AVSampleFormat out_sample_fmt;
	static uint8_t *out_buffer;
	static int out_buffer_size;
	//static int nextSize;
	static int64_t timestamp_last_frame;
	static ManagerPlayer *mPlayer;
	static SDL_AudioSpec wanted_spec;

private:
};


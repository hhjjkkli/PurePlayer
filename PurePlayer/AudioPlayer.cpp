#include "AudioPlayer.h"
#include "ManagerPlayer.h"
#pragma warning(disable: 4996)

Uint32 audio_len = 0;
Uint8 *audio_pos = NULL;
Uint8 *audio_chunk = NULL;

AVSampleFormat AudioPlayer::out_sample_fmt = AV_SAMPLE_FMT_S16;;
uint8_t *AudioPlayer::out_buffer = nullptr;
int AudioPlayer::out_buffer_size = 0;

SDL_AudioSpec AudioPlayer::wanted_spec;
ManagerPlayer *AudioPlayer::mPlayer = nullptr;
int64_t AudioPlayer::timestamp_last_frame = 0;

void callback_audio(void *udata, Uint8 *stream, int len)
{
	SDL_memset(stream, 0, len);
	if (audio_len == 0) {
		return;
	}
	len = (len>audio_len ? audio_len : len);
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int AudioPlayer::play_audio_sdl() {
	mPlayer->wait_state(PlayerState::READY);

	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	uint8_t *out_buffer;
	int nextSize;
	int out_buffer_size = av_samples_get_buffer_size(NULL, mPlayer->audioPlayer->audioDecoder.avctx->channels, mPlayer->audioPlayer->audioDecoder.avctx->frame_size, out_sample_fmt, 1);
	out_buffer = (uint8_t*)av_malloc(out_buffer_size);

	wanted_spec.freq = mPlayer->audioPlayer->audioDecoder.avctx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = mPlayer->audioPlayer->audioDecoder.avctx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = mPlayer->audioPlayer->audioDecoder.avctx->frame_size;
	wanted_spec.callback = callback_audio;
	wanted_spec.userdata = mPlayer->audioPlayer->audioDecoder.avctx;
	////GETCALLERINFO; logd(Log::caller, "before SDL_OpenAudio: format is S16SYS=%d, freq=%d, channels=%d, samples=%d", wanted_spec.format == AUDIO_S16SYS, wanted_spec.freq, wanted_spec.channels, wanted_spec.samples);

	if (SDL_OpenAudio(&wanted_spec, NULL)<0) {
		//GETCALLERINFO; logd(Log::caller, "SDL_OpenAudio() error");
		return 0;
	}
	////GETCALLERINFO; logd(Log::caller, "after SDL_OpenAudio: format is S16SYS=%d, freq=%d, channels=%d, samples=%d", wanted_spec.format == AUDIO_S16SYS, wanted_spec.freq, wanted_spec.channels, wanted_spec.samples);

	bool start = false;
	while (mPlayer->audioPlayer->get_aud_buffer(out_buffer)) {
		//GETCALLERINFO; logd(Log::caller, "get_aud_buffer");
		while (mPlayer->pause_request) {
			//
		}
		while (audio_len>0) {
			SDL_Delay(1);
		}

		audio_chunk = (Uint8 *)out_buffer;
		audio_len = out_buffer_size;
		audio_pos = audio_chunk;
		////GETCALLERINFO; logd(Log::caller, "main.play_audio_sdl(): audio_len = %d, audio_pos = %lld", audio_len, audio_pos);

		if (start == false) {
			//GETCALLERINFO; logd(Log::caller, "SDL_PauseAudio(0)");
			SDL_PauseAudio(0);
			start = true;
		}
	}

	return 0;
}

AudioPlayer::AudioPlayer(ManagerPlayer *mPlayer)
{
	this->mPlayer = mPlayer;
}

AudioPlayer::~AudioPlayer()
{
}

bool start = false;
void AudioPlayer::play_current_frame()
{
	mPlayer->audioPlayer->get_aud_buffer(out_buffer);
	while (mPlayer->pause_request) {
		//
	}
	while (audio_len>0) {
		SDL_Delay(1);
	}

	audio_chunk = (Uint8 *)out_buffer;
	audio_len = out_buffer_size;
	audio_pos = audio_chunk;
	////GETCALLERINFO; logd(Log::caller, "main.play_audio_sdl(): audio_len = %d, audio_pos = %lld", audio_len, audio_pos);

	if (start == false) {
		//GETCALLERINFO; logd(Log::caller, "SDL_PauseAudio(0)");
		SDL_PauseAudio(0);
		start = true;
	}
}

bool AudioPlayer::get_aud_buffer(uint8_t *outputBuffer) {
	if (outputBuffer == nullptr) return false;
	auto av_frame = this->audioDecoder.frame_queue.get_frame((char*)"audio");

	av_samples_get_buffer_size(NULL, this->audioDecoder.avctx->channels, this->audioDecoder.avctx->frame_size, this->audioDecoder.avctx->sample_fmt, 1);

	int ret = swr_convert(ManagerPlayer::swr_ctx, &outputBuffer, this->audioDecoder.avctx->frame_size,
		(uint8_t const **)(av_frame->frame->extended_data),
		av_frame->frame->nb_samples);
	int64_t timestamp = av_frame->frame->pkt_pts; //* av_q2d(ManagerPlayer::streamOfAudio->time_base);
	ManagerPlayer::audio_clock = av_frame->frame->pkt_pts;
	int64_t time_wait = av_gettime_relative() + (timestamp - this->timestamp_last_frame) * ManagerPlayer::rate_play;
	//GETCALLERINFO; logd(Log::caller, "AudioPlayer::get_aud_buffer(): timestamp=%lf, duration=%lf", timestamp, av_frame->duration);
	this->timestamp_last_frame = timestamp;
	while (av_gettime_relative() < time_wait) {
		//GETCALLERINFO; logd(Log::caller, "AudioPlayer::get_aud_buffer(): wait clock, %ld", time_wait - av_gettime_relative());
	}
	av_frame_free(&av_frame->frame);

	return ret >= 0;
}

void AudioPlayer::prepare()
{
	mPlayer->wait_state(PlayerState::READY);

	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	uint8_t *out_buffer;
	int nextSize;
	int out_buffer_size = av_samples_get_buffer_size(NULL, mPlayer->audioPlayer->audioDecoder.avctx->channels, mPlayer->audioPlayer->audioDecoder.avctx->frame_size, out_sample_fmt, 1);
	out_buffer = (uint8_t*)av_malloc(out_buffer_size);

	wanted_spec.freq = mPlayer->audioPlayer->audioDecoder.avctx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = mPlayer->audioPlayer->audioDecoder.avctx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = mPlayer->audioPlayer->audioDecoder.avctx->frame_size;
	wanted_spec.callback = callback_audio;
	wanted_spec.userdata = mPlayer->audioPlayer->audioDecoder.avctx;
	//GETCALLERINFO; logd(Log::caller, "before SDL_OpenAudio: format is S16SYS=%d, freq=%d, channels=%d, samples=%d", wanted_spec.format == AUDIO_S16SYS, wanted_spec.freq, wanted_spec.channels, wanted_spec.samples);

	if (SDL_OpenAudio(&wanted_spec, NULL)<0) {
		//GETCALLERINFO; logd(Log::caller, "SDL_OpenAudio() error");
		return;
	}
	//GETCALLERINFO; logd(Log::caller, "after SDL_OpenAudio: format is S16SYS=%d, freq=%d, channels=%d, samples=%d", wanted_spec.format == AUDIO_S16SYS, wanted_spec.freq, wanted_spec.channels, wanted_spec.samples);

	
}

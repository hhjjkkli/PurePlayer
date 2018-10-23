#include "VideoPlayer.h"
#include "ManagerPlayer.h"
#include "TextureManager.h"
#pragma warning(disable: 4996)

int64_t VideoPlayer::timestamp_last_frame = 0;
SDL_Rect VideoPlayer::videoSrcRect = { 0,0,100,100 };
SDL_Rect VideoPlayer::videoDestRect = { 0,0,100,100 };
VideoDecoder VideoPlayer::videoDecoder;
double VideoPlayer::videoProportion = 1;
AVFrame *VideoPlayer::frameYUV = nullptr;
ManagerPlayer *VideoPlayer::mPlayer = nullptr;

VideoPlayer::VideoPlayer(ManagerPlayer *mPlayer)
{
	this->mPlayer = mPlayer;
}


VideoPlayer::~VideoPlayer()
{
}

bool VideoPlayer::get_img_frame(AVFrame *frame) {
	GETCALLERINFO; logd(Log::caller, "");
	if (frame == nullptr) {
		//GETCALLERINFO; logd(Log::caller, "Player.get_img_frame: frame is null");
		return false;
	}
	auto av_frame = videoDecoder.frame_queue.get_frame((char*)"video");
	////GETCALLERINFO; logd(Log::caller, "Player.get_img_frame: got a frame from viddec.frame_queue.get_frame()");
	////GETCALLERINFO; logd(Log::caller, "Player.get_img_frame: framesize=%d", videoDecoder.frame_queue.get_size());
	sws_scale(ManagerPlayer::img_convert_ctx, (const uint8_t* const*)av_frame->frame->data, av_frame->frame->linesize, 0, videoDecoder.avctx->height,
		frame->data, frame->linesize);
	int64_t timestamp = av_frame_get_best_effort_timestamp(av_frame->frame);//*av_q2d(ManagerPlayer::streamOfVideo->time_base);
	ManagerPlayer::video_clock = av_frame->frame->pkt_pts;
	//之前错在减法做反了
	int64_t time_wait = av_gettime_relative() + (timestamp - this->timestamp_last_frame)  * ManagerPlayer::rate_play;
	this->timestamp_last_frame = timestamp;
	//GETCALLERINFO; logd(Log::caller, "get_img_frame: timestamp=%ld, duration=%ld", timestamp, av_frame->duration);

	while (av_gettime_relative() < time_wait) {
		//GETCALLERINFO; logd(Log::caller, "get_img_frame: wait clock");
	}
	av_frame_free(&av_frame->frame);
	return true;
}

void VideoPlayer::play_current_frame()
{
	if (mPlayer->pause_request == false) {
		mPlayer->videoPlayer->get_img_frame(frameYUV);
	}
	////GETCALLERINFO; logd(Log::caller, "VideoPlayer.show_frame_sdl(): next SDL_UpdateTexture(),framesize=%d", mPlayer->videoPlayer->videoDecoder.frame_queue.get_size());

	SDL_RenderClear(ManagerPlayer::renderer);
	//SDL_Texture *tex = TextureManager::LoadTexture("assets/play.bmp");
	//TextureManager::Draw(tex, NULL, NULL, SDL_FLIP_NONE);

	SDL_UpdateTexture(ManagerPlayer::videoTexture, NULL, frameYUV->data[0], frameYUV->linesize[0]);
	SDL_RenderCopy(ManagerPlayer::renderer, ManagerPlayer::videoTexture, NULL, &VideoPlayer::videoDestRect);

	ManagerPlayer::updateUI();
	mPlayer->drawUI();

	SDL_RenderPresent(ManagerPlayer::renderer);
}

int VideoPlayer::show_frame_sdl(void* mp) {
	//prepare();
	mPlayer->wait_state(PlayerState::READY);

	VideoPlayer::videoProportion = mPlayer->videoPlayer->videoDecoder.get_width() / (double)mPlayer->videoPlayer->videoDecoder.get_height();
	frameYUV = av_frame_alloc();
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, mPlayer->videoPlayer->videoDecoder.get_width(), mPlayer->videoPlayer->videoDecoder.get_height(), 1);
	//GETCALLERINFO; logd(Log::caller, "VideoPlayer::show_frame_sdl(): numBytes=%d", numBytes);
	uint8_t *OutBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	av_image_fill_arrays(frameYUV->data, frameYUV->linesize, OutBuffer, AV_PIX_FMT_YUV420P, mPlayer->videoPlayer->videoDecoder.get_width(), mPlayer->videoPlayer->videoDecoder.get_height(), 1);

	//如果只是简简单单地设置RESIZABLE，是不能SetWindowSize的，会失败卡住，还必须PullEvent进行事件处理循环2018/4/10/2/57
	//SDL_SetWindowSize(ManagerPlayer::window, 1280, 720);

	ManagerPlayer::videoTexture = SDL_CreateTexture(ManagerPlayer::renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		mPlayer->videoPlayer->videoDecoder.get_width(), mPlayer->videoPlayer->videoDecoder.get_height());
	// GETCALLERINFO; logd("ManagerPlayer::window=%x.", ManagerPlayer::window);
	//SDL_SetWindowSize(ManagerPlayer::window, static_cast<int>(VideoPlayer::videoProportion * 360), 360);
	GETCALLERINFO; logd(Log::caller, "VideoPlayer.show_frame_sdl(): next into a loop of get_img_frame()");
	while (mPlayer->videoPlayer->get_img_frame(frameYUV)) {
		//GETCALLERINFO; logd(Log::caller, "on loop...");
		while (mPlayer->pause_request == true) {
			SDL_Delay(40);
			//GETCALLERINFO; logd(Log::caller, "VideoPlayer.show_frame_sdl(): next SDL_UpdateTexture(),framesize=%d", mPlayer->videoPlayer->videoDecoder.frame_queue.get_size());

			SDL_RenderClear(ManagerPlayer::renderer);
			//SDL_Texture *tex = TextureManager::LoadTexture("assets/play.bmp");
			//TextureManager::Draw(tex, NULL, NULL, SDL_FLIP_NONE);

			SDL_UpdateTexture(ManagerPlayer::videoTexture, NULL, frameYUV->data[0], frameYUV->linesize[0]);
			SDL_RenderCopy(ManagerPlayer::renderer, ManagerPlayer::videoTexture, NULL, NULL);

			ManagerPlayer::updateUI();
			mPlayer->drawUI();

			SDL_RenderPresent(ManagerPlayer::renderer);
		}
		////GETCALLERINFO; logd(Log::caller, "VideoPlayer.show_frame_sdl(): next SDL_UpdateTexture(),framesize=%d", mPlayer->videoPlayer->videoDecoder.frame_queue.get_size());

		SDL_RenderClear(ManagerPlayer::renderer);
		//SDL_Texture *tex = TextureManager::LoadTexture("assets/play.bmp");
		//TextureManager::Draw(tex, NULL, NULL, SDL_FLIP_NONE);

		SDL_UpdateTexture(ManagerPlayer::videoTexture, NULL, frameYUV->data[0], frameYUV->linesize[0]);
		SDL_RenderCopy(ManagerPlayer::renderer, ManagerPlayer::videoTexture, NULL, NULL);

		ManagerPlayer::updateUI();
		mPlayer->drawUI();

		SDL_RenderPresent(ManagerPlayer::renderer);
	}

	return 0;
}

void VideoPlayer::handle_resize_window_size(int w, int h) {
	ManagerPlayer::width_window = w;
	ManagerPlayer::height_window = h;
	VideoPlayer::adjustVideoTexture(w, h);
	SDL_DestroyWindow(ManagerPlayer::window);
	SDL_DestroyRenderer(ManagerPlayer::renderer);
	SDL_DestroyTexture(ManagerPlayer::videoTexture);

	ManagerPlayer::window = SDL_CreateWindow(ManagerPlayer::window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_RESIZABLE);
	if (ManagerPlayer::window)
	{
		//GETCALLERINFO; logd(Log::caller, "Window created!");
	}

	ManagerPlayer::renderer = SDL_CreateRenderer(ManagerPlayer::window, -1, 0);
	if (ManagerPlayer::renderer)
	{
		//GETCALLERINFO; logd(Log::caller, "Renderer created!");
	}

	ManagerPlayer::videoTexture = SDL_CreateTexture(ManagerPlayer::renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		videoDecoder.get_width(), videoDecoder.get_height());

	//SDL_SetRenderDrawColor(ManagerPlayer::renderer, 0xff, 0xff, 0xff, 0xff);//white
	ManagerPlayer::isWindowChanged = true;
}

void VideoPlayer::adjustVideoTexture(int w, int h) {
	double proportion = w / (double)h;

	double diff = proportion - VideoPlayer::videoProportion;
	double x1 = 0, y1 = 0, w1, h1;

	if (0 == diff) {
		VideoPlayer::videoDestRect = { 0, 0, w, h };
	}
	else if (diff > 0) {
		y1 = 0; h1 = h;
		w1 = h * VideoPlayer::videoProportion;
		x1 = (w - w1) / 2;
		VideoPlayer::videoDestRect = { (int)x1, (int)y1, (int)w1, (int)h1 };
	}
	else {
		x1 = 0; w1 = w;
		h1 = w / VideoPlayer::videoProportion;
		y1 = (h - h1) / 2;
		VideoPlayer::videoDestRect = { (int)x1, (int)y1, (int)w1, (int)h1 };
	}
}

void VideoPlayer::prepare() {
	mPlayer->wait_state(PlayerState::READY);

	VideoPlayer::videoProportion = mPlayer->videoPlayer->videoDecoder.get_width() / (double)mPlayer->videoPlayer->videoDecoder.get_height();
	frameYUV = av_frame_alloc();
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, mPlayer->videoPlayer->videoDecoder.get_width(), mPlayer->videoPlayer->videoDecoder.get_height(), 1);
	//GETCALLERINFO; logd(Log::caller, "VideoPlayer::show_frame_sdl(): numBytes=%d", numBytes);
	uint8_t *OutBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	av_image_fill_arrays(frameYUV->data, frameYUV->linesize, OutBuffer, AV_PIX_FMT_YUV420P, mPlayer->videoPlayer->videoDecoder.get_width(), mPlayer->videoPlayer->videoDecoder.get_height(), 1);

	//如果只是简简单单地设置RESIZABLE，是不能SetWindowSize的，会失败卡住，还必须PullEvent进行事件处理循环2018/4/10/2/57
	//SDL_SetWindowSize(ManagerPlayer::window, 1280, 720);

	ManagerPlayer::videoTexture = SDL_CreateTexture(ManagerPlayer::renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		mPlayer->videoPlayer->videoDecoder.get_width(), mPlayer->videoPlayer->videoDecoder.get_height());
	SDL_SetWindowSize(ManagerPlayer::window, static_cast<int>(VideoPlayer::videoProportion * 360), 360);
	//如果没有重建窗口，调用这个函数会导致无法绘图
}

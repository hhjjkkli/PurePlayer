#include "ManagerPlayer.h"
#include "ManagerUI.h"
#include "Button.h"

double ManagerPlayer::rate_play = 1;
bool ManagerPlayer::pause_request = false;
int ManagerPlayer::last_motion_x = 0;
int ManagerPlayer::last_motion_y = 0;
int64_t ManagerPlayer::last_click_time = 0;
int64_t ManagerPlayer::last_emotion_time = 0;
char ManagerPlayer::assets[][100] = {"assets/playing.bmp", "assets/play.bmp", "assets/ratedown.bmp", "assets/rateup.bmp"};
char* ManagerPlayer::window_title = (char*)"Pure Media Player";
int ManagerPlayer::width_window = 0;
int ManagerPlayer::height_window = 0;
bool ManagerPlayer::isWindowChanged = false;
ManagerUI *ManagerPlayer::managerUI = nullptr;
int64_t ManagerPlayer::video_clock = 0;
int64_t ManagerPlayer::audio_clock = 0;
SDL_Event ManagerPlayer::event;
bool ManagerPlayer::isRunning = false;
SDL_Renderer *ManagerPlayer::renderer = nullptr;
SDL_Texture *ManagerPlayer::videoTexture = nullptr;
struct SwsContext *ManagerPlayer::img_convert_ctx = nullptr;
struct SwrContext *ManagerPlayer::swr_ctx = nullptr;
AVStream *ManagerPlayer::streamOfAudio = nullptr;
AVStream *ManagerPlayer::streamOfVideo = nullptr;
SDL_Window *ManagerPlayer::window = nullptr;

void ManagerPlayer::handleEvent()
{
	Uint32 windowID = SDL_GetWindowID(window);

	SDL_PollEvent(&event);
	switch (event.type) {
	case SDL_QUIT:
		isRunning = false;
		break;
	case SDL_MOUSEBUTTONDOWN:
		this->last_emotion_time = SDL_GetTicks();
		managerUI->handleMouseEvent(event);
		break;
	case SDL_MOUSEMOTION:
		if (event.button.x == last_motion_x && event.button.y == last_motion_y)
			break;
		this->last_motion_x = event.button.x;
		this->last_motion_y = event.button.y;
		if (isShowCursor == false) {
			SDL_ShowCursor(1);
			isShowCursor = true;
			this->last_emotion_time = SDL_GetTicks();		
		}
		
		break;
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE:
		case SDLK_q:
			isRunning = false;
			break;
		case SDLK_p:
			togglePause();
			break;
		}
		break;
	case SDL_WINDOWEVENT:
		if (event.window.windowID == windowID) {
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				//width = event.window.data1;
				//height = event.window.data2;
				VideoPlayer::handle_resize_window_size(event.window.data1, event.window.data2);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				event.type = SDL_QUIT;
				SDL_PushEvent(&event);
				break;
			}
		}
		break;
	default:
		break;
	}
}

ManagerPlayer::ManagerPlayer()
{
	this->managerUI = new ManagerUI(this);
}

void ManagerPlayer::init(const std::string file_path, char * title, int xpos, int ypos, int width, int height, bool fullscreen)
{
	this->window_title = title;

	int flags = SDL_WINDOW_RESIZABLE;
	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		//GETCALLERINFO; logd(Log::caller, "Subsystem Initialised!");

		window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (window)
		{
			//GETCALLERINFO; logd(Log::caller, "Window created!");
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer)
		{
			//GETCALLERINFO; logd(Log::caller, "Renderer created!");
		}

		isRunning = true;
	}
	else
	{
		isRunning = false;
	}

	av_register_all();
	avformat_network_init();
	av_log_set_flags(AV_LOG_SKIP_REPEATED);

	if (file_path.empty())
	{
		return;
	}
	this->file_path = av_strdup(file_path.c_str());

	this->videoPlayer = new VideoPlayer(this);
	this->audioPlayer = new AudioPlayer(this);

	std::thread read_thread(&ManagerPlayer::read_file, this);
	read_thread.detach();

	managerUI->initUI();
}

void ManagerPlayer::update()
{
}

void ManagerPlayer::render()
{
	//SDL_RenderClear(renderer);
	//draw texture
	//SDL_RenderPresent(renderer);
}

void ManagerPlayer::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	//GETCALLERINFO; logd(Log::caller, "Game cleaned!");

	exit(0);
}

void ManagerPlayer::read_file()
{
	int i, ret;
	AVPacket *pkt = (AVPacket*)av_malloc(sizeof(AVPacket));
	AVFormatContext *t_format_ctx = avformat_alloc_context();
	this->format_ctx = t_format_ctx;
	if (!t_format_ctx)
	{
		//GETCALLERINFO; logd(Log::caller, "error: t_format_ctx = avformat_alloc_context();");
		return;
	}
	ret = avformat_open_input(&t_format_ctx, this->file_path, NULL, NULL);
	//GETCALLERINFO; logd(Log::caller, "avformat_open_input(&t_format_ctx, %s, NULL, NULL);", this->file_path);
	if (ret < 0)
	{
		return;
	}

	ret = avformat_find_stream_info(this->format_ctx, NULL);
	//GETCALLERINFO; logd(Log::caller, "avformat_find_stream_info(this->format_ctx, NULL);");
	if (ret < 0) {
		//GETCALLERINFO; logd(Log::caller, "%s could not find codec parameters", file_path);
	}

	for (i = 0; i < t_format_ctx->nb_streams; i++)
	{
		if (t_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			this->stream_index[AVMEDIA_TYPE_VIDEO] = i;
			read_stream(i);
		}
		else if (t_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			this->stream_index[AVMEDIA_TYPE_AUDIO] = i;
			read_stream(i);
		}
		else if (t_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
		{
			this->stream_index[AVMEDIA_TYPE_SUBTITLE] = i;
			read_stream(i);
		}
	}
	if (this->stream_index[AVMEDIA_TYPE_AUDIO] < 0 &&
		this->stream_index[AVMEDIA_TYPE_VIDEO] < 0)
	{
		//GETCALLERINFO; logd(Log::caller, "Do not have any data in the file");
		return;
	}

	while (true)
	{
		if (true == this->abort_request)
		{
			break;
		}
		while (true == this->pause_request)
		{
			//
		}
		if (true == this->seek_request)
		{
			player_seek();
		}
		ret = av_read_frame(t_format_ctx, pkt);
		if (ret < 0)
		{
			if ((ret == AVERROR_EOF || avio_feof(format_ctx->pb)) && false == this->eof)
			{
				GETCALLERINFO; logd(Log::caller, "***************************** packet decode thread put_nullpacket **************************");
				if (this->stream_index[AVMEDIA_TYPE_VIDEO] >= 0)
					this->videoPlayer->videoDecoder.pkt_queue.put_nullpacket(AVMEDIA_TYPE_VIDEO);
				if (this->stream_index[AVMEDIA_TYPE_AUDIO] >= 0)
					this->audioPlayer->audioDecoder.pkt_queue.put_nullpacket(AVMEDIA_TYPE_AUDIO);
				this->eof = true;
			}
			if (t_format_ctx->pb && t_format_ctx->pb->error) {
				break;
			}
		}
		else
		{
			eof = false;
		}

		if (pkt->stream_index == this->stream_index[AVMEDIA_TYPE_AUDIO])
		{

			if (audioPlayer->audioDecoder.pkt_queue.put_packet(AVMEDIA_TYPE_AUDIO, pkt) < 0)
			{
				break;
			}
			// GETCALLERINFO; logd(Log::caller, "put one packet to audio queue. size=%d. audio frame queue size=%d.", audioPlayer->audioDecoder.pkt_queue.get_queue_size(), audioPlayer->audioDecoder.frame_queue.get_size());
		}
		else if (pkt->stream_index == this->stream_index[AVMEDIA_TYPE_VIDEO])
		{
			ret = videoPlayer->videoDecoder.pkt_queue.put_packet(AVMEDIA_TYPE_VIDEO, pkt);

			if (ret < 0)
			{
				break;
			}
			//GETCALLERINFO; logd(Log::caller, "videoDecoder put one packet.");
		}
		else
		{
			av_packet_unref(pkt);
		}
	}
	GETCALLERINFO; logd(Log::caller, "***************************** packet decode thread break **************************");
}

int ManagerPlayer::read_stream(int stream_num) {
	int ret = 0;
	if (stream_num < 0 || stream_num >= format_ctx->nb_streams)
		return -1;
	AVCodecContext *avctx;
	AVCodec *codec;
	avctx = avcodec_alloc_context3(NULL);
	//GETCALLERINFO; logd(Log::caller, "avctx = avcodec_alloc_context3(NULL);");
	if (!avctx)
		return -2;

	ret = avcodec_parameters_to_context(avctx, this->format_ctx->streams[stream_num]->codecpar);
	//GETCALLERINFO; logd(Log::caller, "ret = avcodec_parameters_to_context(avctx, this->format_ctx->streams[stream_num]->codecpar);");
	if (ret < 0) {
		avcodec_free_context(&avctx);
		return -3;
	}
	av_codec_set_pkt_timebase(avctx, format_ctx->streams[stream_num]->time_base);
	codec = avcodec_find_decoder(avctx->codec_id);
	//GETCALLERINFO; logd(Log::caller, "codec = avcodec_find_decoder(avctx->codec_id);");
	avctx->codec_id = codec->id;
	this->eof = 0;
	format_ctx->streams[stream_num]->discard = AVDISCARD_DEFAULT;
	ret = avcodec_open2(avctx, codec, NULL);
	//GETCALLERINFO; logd(Log::caller, "ret = avcodec_open2(avctx, codec, NULL);");
	if (ret < 0) {
		avcodec_free_context(&avctx);
		return ret;
	}
	switch (avctx->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		this->swr_ctx = swr_alloc();
		this->swr_ctx = swr_alloc_set_opts(swr_ctx, AV_CH_LAYOUT_STEREO,
			AV_SAMPLE_FMT_S16, avctx->sample_rate,
			av_get_default_channel_layout(avctx->channels), avctx->sample_fmt, avctx->sample_rate,
			0, NULL);
		if (!swr_ctx || swr_init(swr_ctx) < 0) {
			swr_free(&swr_ctx);
			return -1;
		}
		this->streamOfAudio = format_ctx->streams[stream_num];
		this->audioPlayer->audioDecoder.init(avctx);
		this->audioPlayer->audioDecoder.start_decode_thread();
		GETCALLERINFO; logd(Log::caller, "*************** audio decoder started **************.");
		break;
	case AVMEDIA_TYPE_VIDEO:
		this->streamOfVideo = format_ctx->streams[stream_num];
		//GETCALLERINFO; logd(Log::caller, "sws_getContext()");
		//如果第三个参数是AV_PIX_FMT_NONE，就会导致runtime error，花了一天时间找这个问题
		//但是在另一个项目中，都不会出现AV_PIX_FMT_NONE，这里为什么会这样呢？----疏忽之下，没有find_stream_info，导致调试了一天。
		//this function is to be removed after a saner alternative is written
		ManagerPlayer::img_convert_ctx = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
			avctx->width, avctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		//ManagerPlayer::img_convert_ctx = sws_getContext(500, 500, AV_PIX_FMT_BGR0,
		//	500, 500, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		//GETCALLERINFO; logd(Log::caller, "sws_getContext()");
		this->videoPlayer->videoDecoder.init(avctx);
		this->videoPlayer->videoDecoder.start_decode_thread();
		GETCALLERINFO; logd(Log::caller, "*************** video decoder started **************.");
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		break;
	default:
		break;
	}
	//GETCALLERINFO; logd(Log::caller, "Player.open_stream --> Player.change_state(PlayerState::READY)");
	change_state(PlayerState::READY);
	return ret;
}

void ManagerPlayer::change_state(PlayerState state)
{
	std::unique_lock<std::mutex> lock(mutex);
	if (state == PlayerState::READY)
	{
		if (this->audioPlayer->audioDecoder.avctx != NULL && this->videoPlayer->videoDecoder.avctx != NULL)
		{
			this->state = state;
		}
	}
	else
	{
		this->state = state;
	}
	state_condition.notify_all();
}

void ManagerPlayer::togglePause()
{
	std::unique_lock<std::mutex> lock(mutex);
	this->pause_request = !this->pause_request;
	ManagerPlayer::managerUI->buttons[0]->reloadTexture(ManagerPlayer::assets[(int)ManagerPlayer::pause_request]);
}

void ManagerPlayer::toggleSeek(int64_t seekTo)
{
	this->seek_request = true;
	this->seek_timestamp = seekTo;
}

void ManagerPlayer::player_seek()
{
	int ret = av_seek_frame(format_ctx, -1, this->seek_timestamp * AV_TIME_BASE, 1);
	if (ret < 0)
	{
		//GETCALLERINFO; logd(Log::caller, "Error: av_seek_frame()");
	}
	else
	{
		if (this->stream_index[AVMEDIA_TYPE_VIDEO] >= 0) {
			this->videoPlayer->videoDecoder.pkt_queue.flush();
		}
		if (this->stream_index[AVMEDIA_TYPE_AUDIO] >= 0) {
			this->audioPlayer->audioDecoder.pkt_queue.flush();
		}
	}
	this->seek_request = false;
	this->eof = 0;
}

double ManagerPlayer::get_duration() {
	if (this->format_ctx != nullptr) {
		return this->format_ctx->duration / 1000000.0;
	}
}

void ManagerPlayer::wait_state(PlayerState need_state) {
	std::unique_lock<std::mutex> lock(mutex);
	state_condition.wait(lock, [this, need_state] {
		return this->state >= need_state;
	});
}

void ManagerPlayer::drawUI() {
	if (SDL_GetTicks() - this->last_emotion_time < 4000) {
		if (isShowCursor == false) {
			SDL_ShowCursor(1);
			isShowCursor = true;
		}
		managerUI->draw();

	}
	else {
		if (isShowCursor == true) {
			SDL_ShowCursor(0);
			isShowCursor = false;			
		}
	}
}

void ManagerPlayer::updateUI() {
	managerUI->update();
}

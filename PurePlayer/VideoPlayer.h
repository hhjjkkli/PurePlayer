#pragma once
#include "BasePlayer.h"
#include "VideoDecoder.h"
class ManagerPlayer;

class VideoPlayer: public BasePlayer
{
public:
	VideoPlayer(ManagerPlayer* mPlayer);
	~VideoPlayer();
	void play_current_frame();

	bool get_img_frame(AVFrame * frame);
	static int show_frame_sdl(void *);
	static void handle_resize_window_size(int w, int h);

	static void adjustVideoTexture(int w, int h);

	static void prepare();

	static int64_t timestamp_last_frame;
	static VideoDecoder videoDecoder;

	static SDL_Rect videoSrcRect;
	static SDL_Rect videoDestRect;
	static double videoProportion; //width : height
	static ManagerPlayer *mPlayer;

	static AVFrame *frameYUV;
private:
	
};

#pragma once
#include <SDL.h>
#include "VideoPlayer.h"
#include "AudioPlayer.h"
#include "log.h"

class ManagerUI;
class AudioPlayer;

class ManagerPlayer
{
public:
	ManagerPlayer();
	void init(const std::string file_path, char * title, int xpos, int ypos, int width, int height, bool fullscreen);
	~ManagerPlayer() {};

	void handleEvent();
	void update();
	void render();
	void clean();

	void read_file();

	void drawUI();

	static void updateUI();

	int read_stream(int stream_index);

	void change_state(PlayerState state);

	void togglePause();

	void toggleSeek(int64_t seekTo);

	void player_seek();

	double get_duration();

	void wait_state(PlayerState need_state);

	bool running() {
		return isRunning;
	}

	static int64_t last_emotion_time;
	static int width_window;
	static int height_window;
	static bool isWindowChanged;
	static SDL_Renderer *renderer;
	static SDL_Texture *videoTexture;
	static SDL_Event event;
	static bool isRunning;

	char *file_path;
	AVFormatContext *format_ctx;

	bool isShowCursor = true;
	bool abort_request = false;
	bool seek_request = false;
	static bool pause_request;
	bool eof = false;

	PlayerState state = PlayerState::UNKNOWN;	

	static double rate_play;
	static int last_motion_x;
	static int last_motion_y;
	static int64_t last_click_time;
	static char assets[][100];
	static char* window_title;
	static int64_t video_clock;
	static int64_t audio_clock;
	static SDL_Window *window;
	static struct SwsContext *img_convert_ctx;
	static struct SwrContext *swr_ctx;
	static AVStream *streamOfAudio;
	static AVStream *streamOfVideo;
	
	int stream_index[AVMEDIA_TYPE_NB] = { -1 };
	AVCodecContext *avctx;
	AVCodec *codec;
	VideoPlayer *videoPlayer;
	AudioPlayer *audioPlayer;

	static ManagerUI *managerUI;

	int64_t seek_timestamp = 0;
private:
	int64_t timestamp_player = 0;

	std::mutex mutex;
	std::condition_variable state_condition;
};

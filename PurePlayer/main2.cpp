#include <SDL.h>
#include <Windows.h>
#include "log.h"
#include "ManagerPlayer.h"
#include "AudioPlayer.h"
#include "VideoPlayer.h"

#pragma warning(disable:4819)

extern ManagerPlayer *mp;

void videoThreadWrap() {
	mp->videoPlayer->show_frame_sdl((void*)mp);
}
void audioThreadWrap() {
	mp->audioPlayer->play_audio_sdl();
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
)
{
	logc();

	mp = new ManagerPlayer();
	mp->init("DreamItPossible.mp4", (char*)"Pure Media Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, false);
	mp->wait_state(PlayerState::READY);
	
	// std::thread只能传入C函数,或者静态函数
	std::thread videoThread(videoThreadWrap);
	std::thread audioThread(audioThreadWrap);
	videoThread.detach();
	audioThread.detach();
	while (mp->isRunning) {
		SDL_Delay(10);
		mp->handleEvent(); // 因为主线程创建了window,并且init了SDL,所以事件得在主线程管理.
	}

	mp->clean();

	return 0;
}
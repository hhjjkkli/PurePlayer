#pragma once
#include <SDL.h>
#include "ManagerPlayer.h"

class Button
{
public:
	Button(ManagerPlayer* mPlayer, double x, double y, double w, double h, const char* filepath, ButtonFunction fun);
	~Button();

	void update();

	SDL_Rect destRect;

	double x = 0, y = 0, w = 0, h = 0;
	SDL_Texture *texture = nullptr;

	char* filepath;

	void draw();

	void handleMouseEvent(int xpos, int ypos);

	void reloadTexture(char * filename);

	ButtonFunction call_back = ButtonFunction::UNKNOW;
	ManagerPlayer *mPlayer = nullptr;

private:

};


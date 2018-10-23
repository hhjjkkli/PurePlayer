#pragma once
#include <vector>
#include <SDL.h>
#include "UtilityHeader.h"

class ManagerPlayer;
class Button;

class ManagerUI
{
public:
	ManagerUI(ManagerPlayer * mPlayer);
	~ManagerUI();

	void update();

	void draw();

	void initUI();

	void handleMouseEvent(SDL_Event e);

	void addButton(double x, double y, double w, double h, const char * filepath, ButtonFunction fun);

	std::vector<Button*> buttons;

	ManagerPlayer *mPlayer = nullptr;

	SDL_Texture *UIBackgroud = nullptr;

	SDL_Rect destRect;
};

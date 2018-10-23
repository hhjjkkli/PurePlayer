#include "ManagerUI.h"
#include "TextureManager.h"
#include "Button.h"
#include "ManagerPlayer.h"

ManagerUI::ManagerUI(ManagerPlayer* mPlayer)
{
	this->mPlayer = mPlayer;
}


ManagerUI::~ManagerUI()
{
}

void ManagerUI::update() {
	if (ManagerPlayer::isWindowChanged == true) {		
		int i = 0;
		for (i = 0; i < buttons.size(); i++) {
			buttons[i]->update();
		}

		SDL_DestroyTexture(this->UIBackgroud);
		this->UIBackgroud = SDL_CreateTexture(ManagerPlayer::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
			ManagerPlayer::width_window, ManagerPlayer::height_window * 0.2);
		SDL_SetRenderTarget(ManagerPlayer::renderer, this->UIBackgroud);		
		this->destRect = {0, static_cast<int>(ManagerPlayer::height_window * 0.8), ManagerPlayer::width_window,  static_cast<int>(ManagerPlayer::height_window * 0.2) };
		//没注意这里绘图的坐标是texture内部坐标，而不能再用屏幕坐标，导致什么都没有绘制出来 2018/4/12
		SDL_Rect tr = { 0, 0, ManagerPlayer::width_window,  static_cast<int>(ManagerPlayer::height_window * 0.2) };
		SDL_RenderDrawRect(ManagerPlayer::renderer, &tr);
		SDL_SetRenderDrawColor(ManagerPlayer::renderer, 0xff, 0xff, 0xff, 0xff);//white
		SDL_RenderFillRect(ManagerPlayer::renderer, &tr);
		SDL_SetRenderTarget(ManagerPlayer::renderer, NULL);
		SDL_SetRenderDrawColor(ManagerPlayer::renderer, 0x00, 0x00, 0x00, 0xff);//black

		SDL_SetTextureBlendMode(this->UIBackgroud, SDL_BLENDMODE_BLEND);//设置texture透明度模式  
		SDL_SetTextureAlphaMod(this->UIBackgroud, 50);//设置texture透明度

		ManagerPlayer::isWindowChanged = false;
	}
}

void ManagerUI::addButton(double x, double y, double w, double h, const char* filepath, ButtonFunction fun) {	
	Button *btn = new Button(this->mPlayer, x, y, w, h, filepath, fun);
	buttons.push_back(btn);
}

void ManagerUI::draw() {
	TextureManager::Draw(this->UIBackgroud, NULL, &destRect, SDL_FLIP_NONE);
	int i = 0;
	for (i = 0; i < buttons.size(); i++) {
		buttons[i]->draw();
	}
}

void ManagerUI::initUI() { 
	addButton(0.45, 0.9, 0.1, 0.1, ManagerPlayer::assets[(int)ManagerPlayer::pause_request], ButtonFunction::PAUSE);
	addButton(0.35, 0.9, 0.1, 0.1, ManagerPlayer::assets[2], ButtonFunction::RATEDOWN);
	addButton(0.55, 0.9, 0.1, 0.1, ManagerPlayer::assets[3], ButtonFunction::RATEUP);

}

void ManagerUI::handleMouseEvent(SDL_Event e) {
	if (e.button.button == SDL_BUTTON_LEFT &&
		e.button.type == SDL_MOUSEBUTTONDOWN) {
		int i = 0;
		for (i = 0; i < buttons.size(); i++) {
			buttons[i]->handleMouseEvent(e.button.x, e.button.y);
		}
	}
}

#include "Button.h"
#include "TextureManager.h"
#include "log.h"

Button::Button(ManagerPlayer *mPlayer, double x, double y, double w, double h, const char* filepath, ButtonFunction fun)
{
	this->mPlayer = mPlayer;
	this->call_back = fun;
	this->filepath = (char*)filepath;
	reloadTexture(this->filepath);
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	update();
}


Button::~Button()
{
}

void Button::update() {
	//GETCALLERINFO; logd(Log::caller, "Button::update, ManagerPlayer::isWindowChanged=%d", ManagerPlayer::isWindowChanged);
	if (ManagerPlayer::isWindowChanged == true) {
		int ww = ManagerPlayer::width_window, wh = ManagerPlayer::height_window;
		//如下得到的是pixel坐标，和创建窗口时的屏幕坐标不一致，会偏大
		//SDL_GetWindowSize(ManagerPlayer::window, &ww, &wh);
		destRect.x = static_cast<int>(x * ww);
		destRect.y = static_cast<int>(y * wh);
		destRect.w = static_cast<int>(w * ww);
		destRect.h = static_cast<int>(h * wh);

		reloadTexture(this->filepath);		
	}
}

void Button::draw() {
	TextureManager::Draw(texture, NULL, &destRect, SDL_FLIP_NONE);
}

void Button::handleMouseEvent(int xpos, int ypos) {
	////GETCALLERINFO; logd(Log::caller, "xpos:%d, ypos:%d", xpos, ypos);
	if (xpos >= destRect.x && ypos >= destRect.y &&
		xpos <= destRect.x + destRect.w && ypos <= destRect.y + destRect.h) {		
		switch (this->call_back)
		{
		case ButtonFunction::PAUSE:			
			if (SDL_GetTicks() - ManagerPlayer::last_click_time > 200) {
				ManagerPlayer::last_click_time = SDL_GetTicks();
				//GETCALLERINFO; logd(Log::caller, "%d button callback", this->call_back);				
				mPlayer->togglePause();
			}
			break;
		case ButtonFunction::RATEDOWN:
			ManagerPlayer::rate_play -= 0.1;
			//GETCALLERINFO; logd(Log::caller, "%d button callback, ManagerPlayer::rate_play=%f", this->call_back, ManagerPlayer::rate_play);
			break;
		case ButtonFunction::RATEUP:
			ManagerPlayer::rate_play += 0.1;
			//GETCALLERINFO; logd(Log::caller, "%d button callback, ManagerPlayer::rate_play=%f", this->call_back, ManagerPlayer::rate_play);
			break;
		default:
			break;
		}
	}
}

void Button::reloadTexture(char* filename) {
	this->filepath = filename;
	SDL_DestroyTexture(this->texture);
	this->texture = TextureManager::LoadTexture(filename);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);//设置texture透明度模式  
	SDL_SetTextureAlphaMod(texture, 255);//设置texture透明度
}

#pragma once
#include "BasePlayer.h"
#include "SubtitleDecoder.h"

class SubtitlePlayer: public BasePlayer
{
public:
	SubtitlePlayer();
	~SubtitlePlayer();
	void play_current_frame() {};
	

	SubtitleDecoder subtitleDecoder;
private:

};


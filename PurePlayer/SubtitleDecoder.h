#pragma once
#include "BaseDecoder.h"

class SubtitleDecoder :public BaseDecoder
{
public:
	SubtitleDecoder();
	~SubtitleDecoder();
	void play_current_frame() {};
	void start_decode() {};
	int decode_one_packet() { return 0; };
};


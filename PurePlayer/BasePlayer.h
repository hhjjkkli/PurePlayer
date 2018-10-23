#pragma once
#include "UtilityHeader.h"

class BasePlayer
{
public:
	BasePlayer();
	~BasePlayer();
	virtual void play_current_frame() = 0;
	
private:
	int64_t timestamp = 0;
};


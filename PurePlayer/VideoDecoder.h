#ifndef __VideoDecoder_H__
#define __VideoDecoder_H__
#include "BaseDecoder.h"

class VideoDecoder : public BaseDecoder {
public:
	int decode_one_packet();
	void start_decode();	
	int get_width();
	int get_height();
};
#endif

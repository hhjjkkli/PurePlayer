#ifndef __AudioDecoder_H__
#define __AudioDecoder_H__
#include "BaseDecoder.h"

/*
音频流解码器，增加了信道数和采样率的两个相关成员函数
*/
class AudioDecoder : public BaseDecoder { 
public:
	/*
	解码一个packet为多个frame，把这些frame加到BaseDecoder::frame_queue这个队列里去
	返回值：-1表示abort(),pkt_queue.get_packet()失败,pkt.data == NULL；0表示成功解码这个packet
	*/
	int decode_one_packet();
	/*
	Call decode_one_packet() in a loop to decode the audio
	*/
	void start_decode();
	int get_channels();
	int get_sample_rate();
};
#endif // !__AudioDecoder_H__

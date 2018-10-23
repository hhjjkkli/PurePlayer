#ifndef __Decoder_H__
#define __Decoder_H__
#include "FrameQueue.h"
#include "PacketQueue.h"

class BaseDecoder {
public:
	virtual int decode_one_packet() = 0;
	virtual void start_decode() = 0;
	void init(AVCodecContext *ctx);

	void start_decode_thread();

	PacketQueue pkt_queue;
	FrameQueue frame_queue;
	AVCodecContext *avctx = NULL;
protected:
	AVPacket pkt;
	AVPacket pkt_tmp;
	int pkt_serial;
	int finished;
	int packet_pending=0;
	std::condition_variable empty_queue_cond;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
};
#endif // !__Decoder_H__

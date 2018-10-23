#ifndef __VideoDecoder_C__
#define __VideoDecoder_C__
#include "VideoDecoder.h"

int VideoDecoder::decode_one_packet() {
	int ret;

	do {
		if (pkt_queue.get_abort())
			return -1;
		if (!packet_pending) {
			if (pkt_queue.get_packet(AVMEDIA_TYPE_VIDEO, &pkt) < 0) {
				return -1;
				if (pkt.data == NULL) { 
					//GETCALLERINFO; logd(Log::caller, "VideoDecoder::decode_one_packet(): reach eof.\n");
					return -1;
				}
			}

			ret = avcodec_send_packet(avctx, &pkt);
			if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
				//GETCALLERINFO; logd(Log::caller, "VideoDecoder::decode_one_packet(): video avcodec_send_packet error %d.\n", ret);
				break;
			}

			AVFrame *frame = av_frame_alloc();
			ret = avcodec_receive_frame(avctx, frame);
			if (ret < 0 && ret != AVERROR_EOF) {//-11, other negative values: legitimate decoding errors
				////GETCALLERINFO; logd(Log::caller, "VideoDecoder::decode_one_packet(): video avcodec_receive_frame error %d.\n", ret);
				break;
			}
			//double pts;           /* presentation timestamp for the frame */
			frame->pts = av_frame_get_best_effort_timestamp(frame);
			frame_queue.put_frame(frame, (char*)"audio");
			//GETCALLERINFO; logd(Log::caller, "VideoDecoder::decode_one_packet(): put one frame to VideoDecoder.frame_queue,framesize=%d", this->frame_queue.get_size());
			if (ret < 0) {
				packet_pending = 0;
			}
		}
	} while (ret != 0 && !finished);
	return 0;
}

void VideoDecoder::start_decode() {
	while (true) {
		if (pkt_queue.get_abort())
			break;
		int ret;
		if ((ret = decode_one_packet()) < 0) {
			//GETCALLERINFO; logd(Log::caller, "VideoDecoder::start_decode(): decode_one_packet() return <0");
			return;
		}
	}
}

int VideoDecoder::get_width() {
	if (!avctx)
		return 0;
	return avctx->width;
}

int VideoDecoder::get_height() {
	if (!avctx)
		return 0;
	return avctx->height;
}

#endif // !__VideoDecoder_C__

#ifndef __AudioDecoder_C__
#define __AudioDecoder_C__
#include "AudioDecoder.h"

// 没用了
int cnt = 0;

int AudioDecoder::decode_one_packet() {
	int ret;

	do {
		if (pkt_queue.get_abort()) {
			return -1;
		}
		
		if (!packet_pending) {
			if (pkt_queue.get_packet(AVMEDIA_TYPE_AUDIO, &pkt) < 0) {
				return -1;
			}
		}
		if (pkt.data == NULL) {
			//GETCALLERINFO; logd(Log::caller, "reach eof.\n");
			return -1;
		}

		ret = avcodec_send_packet(avctx, &pkt);

		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
			GETCALLERINFO; logd(Log::caller, "reach eof.\n");
			break;
		}
		
		int got_frame = 0;
		cnt = 0;
		while (true) {
			AVFrame *frame = av_frame_alloc();
			ret = avcodec_receive_frame(avctx, frame);//Note that the function will always call *av_frame_unref(frame) before doing anything else.所以，这里出错了，frame作为循环外定义的变量，被清空了，而put进去的不是一个拷贝而只是一个指针
			//GETCALLERINFO; logd(Log::caller, "return %d", ret); // 0和-11交替
			if (ret < 0 && ret != AVERROR_EOF) {
				//GETCALLERINFO; logd(Log::caller, "return %d", ret); // 全是-11
				av_frame_free(&frame);//it will call  av_frame_unref(*frame), so do not need unref here
				return 0;
			}
			frame->pts = av_frame_get_best_effort_timestamp(frame);
			frame_queue.put_frame(frame, (char*)"audio");
			//GETCALLERINFO; logd(Log::caller, "put one frame to AudioDecoder.frame_queue,framesize=%d", pkt_queue.get_queue_size());
		}
		if (ret < 0) {
			packet_pending = 0;
		}		
	} while (ret != 0 && !finished);

	GETCALLERINFO; logd(Log::caller, "out from loop.");
	return 0;
}

void AudioDecoder::start_decode() {
	AVFrame *frame = av_frame_alloc(); //分配到堆，回收在最后进程结束才进行，不过视频解码器没写这个分配内存
	while (true) {
		if (pkt_queue.get_abort())
			break;
		int got;
		if ((got = decode_one_packet()) < 0) {
			GETCALLERINFO; logd(Log::caller, "out from loop. got=%d.", got);
			return;
		}
	}
}
int AudioDecoder::get_channels() {
	return avctx->channels;
}
int AudioDecoder::get_sample_rate() {
	return avctx->sample_rate;
}

#endif // !__AudioDecoder_C__

#ifndef __PacketQueue_C__
#define __PacketQueue_C__
#include "PacketQueue.h"
#include "ManagerPlayer.h"
#include "ManagerPlayer.h"
#pragma warning(disable: 4996) // Deprecation

extern ManagerPlayer *mp;

int PacketQueue::put_packet(int queueID, AVPacket *pkt) {
	char * str;
	if (queueID == AVMEDIA_TYPE_AUDIO) {
		str = (char*)"audio";
		GETCALLERINFO; logd(Log::caller, "will add a packet to %s queue. size=%d. audio frame queue size=%d.", str, queue.size(), mp->audioPlayer->audioDecoder.frame_queue.get_size());
	}
	else {
		str = (char*)"video";
	}

	if (abort_request) {
		return -1;
	}
	int ret;
	std::unique_lock < std::mutex > lock(mutex);
	while (true) {
		if (queue.size() < MAX_SIZE) {
			//GETCALLERINFO; logd(Log::caller, "queue.size()=%d < MAX_SIZE", queue.size());
			AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
			if (packet == NULL) {
				//GETCALLERINFO; logd(Log::caller, "PacketQueue::put_packet: av_malloc() error");
				return -1;
			}
			////GETCALLERINFO; logd(Log::caller, "main_play.cpp:53, av_copy_packet(packet, pkt) in");
			ret = av_copy_packet(packet, pkt);
			////GETCALLERINFO; logd(Log::caller, "main_play.cpp:55, av_copy_packet(packet, pkt) out");
			if (ret != 0) {
				//GETCALLERINFO; logd(Log::caller, "PacketQueue::put_packet: av_copy_packet() error");
				return -1;
			}
			queue.push(*packet);
			if (queueID == AVMEDIA_TYPE_AUDIO) {
				//GETCALLERINFO; logd(Log::caller, "add a packet to %s queue. size=%d. audio frame queue size=%d.", str, queue.size(), mp->audioPlayer->audioDecoder.frame_queue.get_size());
			}
			duration += packet->duration;
			cond.notify_one();
			break;
		}
		else {
			GETCALLERINFO; logd(Log::caller, "%s packet queue is full. size=%d. addr=%x.", str, get_queue_size(), &this->mutex);
			full.wait(lock);
			GETCALLERINFO; logd(Log::caller, "%s packet queue after wait, full got notify_one 's information. size=%d. addr=%x.", str, get_queue_size(), &this->mutex);
		}
		// GETCALLERINFO; logd(Log::caller, "%s PacketQueue loop. size=%d.", str, get_queue_size());
	}
	return 0;
}
int PacketQueue::get_packet(int queueID, AVPacket *pkt) {
	char * str;
	if (queueID == AVMEDIA_TYPE_AUDIO) {
		str = (char*)"audio";
		GETCALLERINFO; logd(Log::caller, "will remove a packet from %s queue. size=%d. audio frame queue size=%d. called notify_one.", str, queue.size(), mp->audioPlayer->audioDecoder.frame_queue.get_size());
	}
	else {
		str = (char*)"video";
	}
	std::unique_lock < std::mutex > lock(this->mutex); //之前是mutex,则使用了FrameQueue.h的mutex?
	while(true){
		if (abort_request) {
			return -1;
		}
		if (queue.size() > 0) {
			AVPacket tmp = queue.front();
			//GETCALLERINFO; logd(Log::caller, "main_play.cpp:78, av_copy_packet");
			av_copy_packet(pkt, &tmp);
			//GETCALLERINFO; logd(Log::caller, "main_play.cpp:80, av_copy_packet success one");
			duration -= tmp.duration;
			queue.pop();
			av_packet_unref(&tmp);
			av_free_packet(&tmp);
			
			full.notify_one();
			if (queueID == AVMEDIA_TYPE_AUDIO) {
				//GETCALLERINFO; logd(Log::caller, "remove a packet from %s queue. size=%d. audio frame queue size=%d. called notify_one.", str, queue.size(), mp->audioPlayer->audioDecoder.frame_queue.get_size());
			}
			return 0;
		}
		else {
			GETCALLERINFO; logd(Log::caller, "%s packet queue is empty. size=%d. addr=%x.", str, get_queue_size(), &this->mutex);
			cond.wait(lock);
			GETCALLERINFO; logd(Log::caller, "%s packet queue after wait, empty got notify_one 's information. size=%d. addr=%x.", str, get_queue_size(), &this->mutex);
		}
		// GETCALLERINFO; logd(Log::caller, "%s PacketQueue loop. size=%d.", str, get_queue_size());
	}
}

int PacketQueue::put_nullpacket(int queueID) {
	AVPacket *pkt = new AVPacket();
	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;
	put_packet(queueID, pkt);
	return 0;
}
void PacketQueue::set_abort(int abort) {
	abort_request = abort;
}
int PacketQueue::get_abort() {
	return abort_request;
}

void PacketQueue::flush() {
	std::unique_lock < std::mutex > lock(mutex);
	while (queue.size() > 0) {
		AVPacket tmp = queue.front();
		queue.pop();
		av_packet_unref(&tmp);
		av_free_packet(&tmp); //官网现在写的是void av_packet_free(AVPacket **pkt)
	}
	duration = 0;
	full.notify_one(); //为什么只唤醒一个
}
size_t PacketQueue::get_queue_size() {
	return queue.size();
}

#endif

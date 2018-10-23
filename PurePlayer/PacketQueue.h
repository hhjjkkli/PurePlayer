#ifndef __PacketQueue_H__
#define __PacketQueue_H__
#include "UtilityHeader.h"

class PacketQueue {
public:
	int put_packet(int queueID, AVPacket *pkt);
	int get_packet(int queueID, AVPacket * pkt);
	int put_nullpacket(int queueID);
	void set_abort(int abort);
	int get_abort();
	void flush();
	size_t get_queue_size();
private:
	std::queue<AVPacket> queue;
	int64_t duration;
	int abort_request = 0;
	std::mutex mutex;
	std::condition_variable cond;
	std::condition_variable full;
	const size_t MAX_SIZE = 16;
};
#endif // !__PacketQueue_H__


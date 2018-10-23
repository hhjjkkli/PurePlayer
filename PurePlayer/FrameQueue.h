#ifndef __FrameQueue_H__
#define __FrameQueue_H__
#include "Frame.h"

class FrameQueue {
public:
	void put_frame(AVFrame * frame, char * who);
	std::shared_ptr<Frame> get_frame(char* who);
	size_t get_size();
private:
	std::queue<std::shared_ptr<Frame>> queue;
	std::mutex mutex;
	std::condition_variable empty;
	std::condition_variable full;
	const size_t MAX_SIZE = 16;
};
#endif // !__FrameQueue_H__

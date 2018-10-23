#ifndef __FrameQueue_C__
#define __FrameQueue_C__
#include "FrameQueue.h"
#include "ManagerPlayer.h"

extern ManagerPlayer *mp;

void FrameQueue::put_frame(AVFrame *frame, char* who) {
	std::unique_lock < std::mutex > lock(mutex);
	while (true) {
		if (queue.size() < MAX_SIZE) {
			//这里放入队列的指针还没有具体指向的地址，放入的只是指针本身的地址，也可能是指向指针的指针的地址
			auto m_frame = std::make_shared < Frame >(frame);
			if (!queue.empty()) {
				auto last_frame = queue.back();
				last_frame->duration = frame->pts - last_frame->pts;
			}
			queue.push(m_frame);
			empty.notify_one();
			return;
		}
		else {
			GETCALLERINFO; logd(Log::caller, "%s frame queue is full. size=%d. audio packet queue size=%d. addr=%x.", who, get_size(), mp->audioPlayer->audioDecoder.pkt_queue.get_queue_size(), &this->mutex);
			/*
			原子地释放 lock ，阻塞当前执行线程，并将它添加到于 *this 上等待的线程列表。
			线程将在执行 notify_all() 或 notify_one() 时被解除阻塞。解阻塞时，无关乎原因， 
			lock 得到释放且 wait 退出。若此函数通过异常退出，则亦会重获得 lock 。 (C++14 前)
			*/
			full.wait(lock);
		}
	}
}
std::shared_ptr<Frame> FrameQueue::get_frame(char * who)
{
	GETCALLERINFO; logd(Log::caller, "will remove a frame from %s frame queue. size=%d. video packet queue size=%d.", who, get_size(), mp->videoPlayer->videoDecoder.pkt_queue.get_queue_size());
	// 锁定关联互斥。等效地调用 mutex()->lock() 。
	std::unique_lock < std::mutex > lock(mutex);
	while (true) { // 实际上循环是没有意义的,wait自身其实就会阻塞了
		if (queue.size() > 0) {
			//出队列的指针没必要手动回收内存，指针指向的Frame的内存由系统管理，c++吸收了java的优点
			auto tmp = queue.front();
			queue.pop();
			// GETCALLERINFO; logd(Log::caller, "%s packet queue size=%d. frame queue size=%d.", who, mp->audioPlayer->audioDecoder.pkt_queue.get_queue_size(), get_size());
			full.notify_one(); // 这会使得mutex被释放.

			return tmp;//missed return
		}
		else {
			GETCALLERINFO; logd(Log::caller, "%s frame queue is empty. size=%d. audio packet queue size=%d.", who, get_size(), mp->audioPlayer->audioDecoder.pkt_queue.get_queue_size());
			empty.wait(lock);
		}
		// wait之后,就不会打印下面的log了,说明while循环也卡死
		// GETCALLERINFO; logd(Log::caller, "%s FrameQueue is empty. size=%d.", who, get_size());
	}
}

size_t FrameQueue::get_size() {
	return queue.size();
}
#endif // !__FrameQueue_C__

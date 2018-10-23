#ifndef __Decoder_C__
#define __Decoder_C__
#include "BaseDecoder.h"

/*
指针的值传递，指针本身深拷贝，但是指针指向的AVCodecContext只是浅拷贝，不过由于这个ctx分配在堆上，故而可以使用浅拷贝
*/
void BaseDecoder::init(AVCodecContext *ctx) {
	avctx = ctx; 
}

/*
Create a new thread to execute the BaseDecoder::start_decode() which decode the data in a loop
*/
void BaseDecoder::start_decode_thread() {
	pkt_queue.set_abort(0); //置0才能使用queue
	std::thread t(&BaseDecoder::start_decode, this); //这里不用SDL提供的线程函数；decode是虚函数，那么线程内容依据子类的实现
	t.detach(); //不想detach，这样的话他不能伴随主线程的退出而退出怎么办？它会随着进程回收而结束
}

#endif // !__Decoder_C__

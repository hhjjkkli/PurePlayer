#ifndef __Frame_H__
#define __Frame_H__
#include "UtilityHeader.h"

struct Frame {
	Frame(AVFrame *f):frame(f) {
		this->pts = (double)f->pts;
	}
	AVFrame *frame;
	double pts=0; /* presentation timestamp for the frame */
	int64_t duration=0; /* estimated duration of the frame *///ffplay里的double改成int64_t
	int64_t pos=0; /* byte position of the frame in the input file */

	int width;
	int height;
	int format;
};
#endif

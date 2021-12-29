#include "filter.h"

/*

 FIR filter designed with
 http://t-filter.appspot.com
 */

void SampleFilter_init(SampleFilter *f) {
	// TODO: need to pick an impulse response based on some parameter
	f->impulseResponse = (double*) &impulseReponse;
	f->bufferSize = sizeof(impulseReponse) / sizeof(double);

	int i;
	for (i = 0; i < f->bufferSize; ++i)
		f->history[i] = 0;
	f->last_index = 0;
}

void SampleFilter_put(SampleFilter *f, double input) {
	f->history[f->last_index++] = input;
	if (f->last_index == f->bufferSize)
		f->last_index = 0;
}

double SampleFilter_get(SampleFilter *f) {
	double acc = 0;
	int index = f->last_index, i;
	for (i = 0; i < f->bufferSize; ++i) {
		index = index != 0 ? index - 1 : f->bufferSize - 1;
		acc += f->history[index] * f->impulseResponse[i];
	}
	;
	return acc;
}

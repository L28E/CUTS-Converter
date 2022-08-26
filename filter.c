#include "filter.h"

/*

 FIR filter designed with
 http://t-filter.appspot.com

 */

void Filter_init(Filter *f) {	
	f->impulseResponse = (double*) &lowpassImpulse;
	f->bufferSize = sizeof(lowpassImpulse) / sizeof(double);

	int i;
	for (i = 0; i < f->bufferSize; ++i)
		f->history[i] = 0;
	f->last_index = 0;
}

void Filter_put(Filter *f, double input) {
	f->history[f->last_index++] = input;
	if (f->last_index == f->bufferSize)
		f->last_index = 0;
}

double Filter_get(Filter *f) {
	double acc = 0;
	int index = f->last_index, i;
	for (i = 0; i < f->bufferSize; ++i) {
		index = index != 0 ? index - 1 : f->bufferSize - 1;
		acc += f->history[index] * f->impulseResponse[i];
	}
	;
	return acc;
}

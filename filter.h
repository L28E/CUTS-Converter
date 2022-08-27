#ifndef FILTER_H_
#define FILTER_H_

/*

 FIR filter(s) designed with
 http://t-filter.appspot.com

 */

typedef struct {
	unsigned int last_index;
	unsigned int bufferSize;
	double *impulseResponse;
	double history[];
} Filter;

void Filter_init(Filter *f, double *impulseResponse, unsigned int bufferSize);
void Filter_put(Filter *f, double input);
double Filter_get(Filter *f);

#endif /* FILTER_H_ */

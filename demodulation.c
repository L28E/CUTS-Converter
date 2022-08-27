#include "demodulation.h"

/*
 * Function:  demodulate
 * --------------------
 * Demodulates a CUTS encoded WAV file
 *
 */
int demodulate(char inputFile[]) {
	
	/*
	 * 																	  _____	
	 *																 	 /     \
	 *						.----------------.		 .------------.		 / 	    \
	 *				.------>/  Mark Matched  /------>/  Envelope  /----->/       \
	 *				/		/     Filter     /		 /  Detector  /		 /        \
	 *			 	/		'----------------'		 '------------'		 /   	   \
	 *	FSK input---/ 													 / Decision }----> Binary Out 
	 *				/													 /         /
	 *				/		.----------------.		 .------------.		 /   	  /
	 *				'------>/ Space Matched  /------>/  Envelope  /----->/       /
	 *						/	  Filter	 /		 /  Detector  / 	 /	    /
	 *						'----------------'		 '------------'		 /_____/				
	 *																	 
	 */
		
	FILE *inputFilePtr;
	uint32_t dataSize;
	int16_t *buffer;
	double filterOut;
	Filter *hpf;
	Filter *env;

	// Assuming we've recorded to a cassette, and then digitally recorded the casette back to a wav.
	// Open the wav for reading.
	inputFilePtr = fopen(inputFile, "rb");
	if (inputFilePtr == NULL) {
		return -1;
	}

	/* For now, I'll assume all the parameters of this .wav are the same as the one we recorded to the casette, 
	44100 Hz sample rate, 16 bit PCM
	In that case I just need to get the datasize.
	Note that because this is 16 bit audio i.e. 2 bytes per sample, the number of samples is half the datasize.
	*/
	fseek(inputFilePtr, 40, SEEK_SET);
	fread(&dataSize, 4, 1, inputFilePtr);
	printf("Data size in bytes: %d\n", dataSize);

	// Put all the wav data in a buffer
	buffer = (int16_t*) malloc(dataSize * sizeof(int16_t));
	fread(buffer, dataSize, 1, inputFilePtr);
	fclose(inputFilePtr);

	// Initialize the filters
	hpf = malloc(sizeof(Filter) + sizeof(highpassImpulse));
	env = malloc(sizeof(Filter) + sizeof(envelopeImpulse));
	Filter_init(hpf,highpassImpulse,sizeof(highpassImpulse)/sizeof(double));
	Filter_init(env,envelopeImpulse,sizeof(envelopeImpulse)/sizeof(double));	 
	
	// Open a temporary file for plotting
	FILE *plotData = fopen("data.tmp", "w");
	
	// Step through the whole buffer and demodulate it
	for (int i = 0; i < dataSize / 2; i++) {
		// Update the filters
		Filter_put(hpf, buffer[i]);		

		// Update the envelope detectors
		filterOut=Filter_get(hpf);
		Filter_put(env, abs(filterOut));

		// Compare and decide
		
		// write to the temp file as we go 
		fprintf(plotData, "%d %d %f %f\n", i, buffer[i], filterOut, Filter_get(env));
	}
	
	fclose(plotData);

	FILE *gnuPlotPipe = popen("gnuplot -persistent", "w");
	fprintf(gnuPlotPipe, "%s\n", "plot 'data.tmp' using 1:2 w l title 'raw', '' using 1:3 w l title 'filtered', '' using 1:4 w l title 'env'; pause mouse close");
	fclose(gnuPlotPipe);

	return 0;
}
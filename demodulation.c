#include "demodulation.h"
#include "wavschema.h"

enum cell{MARK=1, SPACE=0};

/*
 * Function:  demodulate
 * --------------------
 * Demodulates a CUTS encoded WAV file. Here's the block diagram:
 *                                                               _____     
 *                                                              /     \
 *                      .-----------.       .------------.      /      \
 *              .------>/  Mark BP  /------>/  Envelope  /----->/       \
 *              /       /  Filter   /       /  Detector  /      /        \
 *              /       '-----------'       '------------'      /         \
 * FSK input----/                                               / Decision }----> Binary Out 
 *              /                                               /         /
 *              /       .-----------.       .------------.      /        /
 *              '------>/  Space LP /------>/  Envelope  /----->/       /
 *                      /   Filter  /       /  Detector  /      /      /
 *                      '-----------'       '------------'      /_____/
 */
int demodulate(char inputFile[]) {		
	FILE *inputFilePtr;
	uint32_t dataSize;
	int16_t *buffer;
	uint8_t k;
	double lpfOut;
	double hpfOut;
	enum cell currentCell;
	Filter *lpf;
	Filter *hpf;
	Filter *spaceEnv;
	Filter *markEnv;

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
	lpf = malloc(sizeof(Filter) + sizeof(spaceImpulse));
	hpf = malloc(sizeof(Filter) + sizeof(markImpulse));
	spaceEnv = malloc(sizeof(Filter) + sizeof(envelopeImpulse));
	markEnv = malloc(sizeof(Filter) + sizeof(envelopeImpulse));
	Filter_init(lpf,spaceImpulse,sizeof(spaceImpulse)/sizeof(double));
	Filter_init(hpf,markImpulse,sizeof(markImpulse)/sizeof(double));
	Filter_init(spaceEnv,envelopeImpulse,sizeof(envelopeImpulse)/sizeof(double));	 
	Filter_init(markEnv,envelopeImpulse,sizeof(envelopeImpulse)/sizeof(double));
	
	// Open a temporary file for plotting
	FILE *plotData = fopen("data.tmp", "w");
	
	// Step through the whole buffer and demodulate it
	k = 0;
	currentCell = MARK;
	for (int i = 0; i < dataSize / 2; i++) {
		// Update the filters
		Filter_put(lpf, buffer[i]);
		Filter_put(hpf, buffer[i]);		

		// Update the envelope detectors
		lpfOut=Filter_get(lpf);
		hpfOut=Filter_get(hpf);
		Filter_put(spaceEnv, abs(lpfOut));
		Filter_put(markEnv, abs(hpfOut));

		// Counter representing Tb, the length of time where each bit is represented. In a 44100 Hz schema each bit is 147 samples long
		k = (k+1)%samplesPerCell; 
		if (k == 0){			
			// Compare and decide
			currentCell = (Filter_get(markEnv)>Filter_get(spaceEnv)) ? MARK : SPACE; 	
		}		
		
		// write to the temp file as we go 
		fprintf(plotData, "%d %d %f %f %f %f %d\n", i, buffer[i], lpfOut, hpfOut, Filter_get(spaceEnv), Filter_get(markEnv), currentCell);

		// TODO: The FSM to decode the bitstream
	}
	
	fclose(plotData);

	FILE *gnuPlotPipe = popen("gnuplot -persistent", "w");
	fprintf(gnuPlotPipe, "%s\n", "plot 'data.tmp' using 1:2 w l title 'raw', '' using 1:3 w l title 'lowpass', '' using 1:4 w l title 'highpass', '' using 1:5 w l title 'lowenv', '' using 1:6 w l title 'highenv', '' using 1:7 w l title 'bitstream'; pause mouse close");
	fclose(gnuPlotPipe);

	return 0;
}

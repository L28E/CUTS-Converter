#include <stdbool.h>

#include "demodulation.h"
#include "wavschema.h"

typedef enum Cell{MARK=0x01, SPACE=0x00} Cell;
typedef enum State{STOP, READ, RSTOP} State;

/*
 * Function:  demodulate
 * --------------------
 * Demodulates a CUTS encoded WAV file. Here's the block diagram:
 *                                                               _____     
 *                                                              /     \
 *                      .-----------.       .------------.      /      \
 *              .------>/  Mark BP  /------>/  Envelope  /__/ __/       \
 *              /       /  Filter   /       /  Detector  /      /        \
 *              /       '-----------'       '------------'      /         \
 * FSK input----/                                               / Decision }----> Binary Out 
 *              /                                               /         /
 *              /       .-----------.       .------------.      /        /
 *              '------>/  Space LP /------>/  Envelope  /__/ __/       /
 *                      /   Filter  /       /  Detector  /      /      /
 *                      '-----------'       '------------'  /   /_____/
 *                                                          /       
 *                                                          Tb      
 */
int demodulate(char inputFile[],char outputFile[],bool plot) {		
	FILE *inputFilePtr;
	FILE *plotData;
	uint32_t dataSize;
	int16_t *buffer;
	uint8_t k = 0;
	uint8_t j = 0;
	double markEnvOut;
	double spaceEnvOut;
	double maxAmpl;
	double thresholdCoeff = 0.4; // The fraction of the max amplitude to use as the threshold.
	double threshold; // A cell must have a greater amplitude than this to be detected. Remember that the envelope filters cause some attentuation
	char byte = 0x00;
	Cell cell = MARK;
	State state = STOP;
	Filter *lpf;
	Filter *hpf;
	Filter *spaceEnv;
	Filter *markEnv;

	// Assuming we've recorded to a cassette, and then digitally recorded the casette back to a wav.
	// Open the wav for reading.
	inputFilePtr = fopen(inputFile, "rb");
	if (inputFilePtr == NULL) {
		printf("Something went wrong opening the input file...\n");
		return -1;
	}

	/* 	
		For now, I'll assume all the parameters of this .wav are the same as the one we recorded to the casette, 
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
	
	if (plot){
		// Open a temporary file for plotting
		plotData = fopen("plot.tmp", "w");
	}	
	
	// Open a file for the bitstream
	FILE *bitstream = fopen(outputFile, "w");

	// Find the maximum amplitude in the buffer and use that to set the threshold for detection
	// TODO: Maybe start searching later in the recording to avoid any pops, clicks, etc.
	maxAmpl=0;
	for (int i = 0; i < dataSize / 2; i++) {
		if(buffer[i]>maxAmpl){
			maxAmpl=buffer[i];
		}
	}
	threshold = thresholdCoeff * maxAmpl;
	
	// Step through the whole buffer and demodulate it	
	for (int i = 0; i < dataSize / 2; i++) {
		
		// Update the filters
		Filter_put(lpf, buffer[i]);
		Filter_put(hpf, buffer[i]);				

		// Update the envelope detectors		
		Filter_put(spaceEnv, abs(Filter_get(lpf)));
		Filter_put(markEnv, abs(Filter_get(hpf)));

		// Counter representing Tb, the length of time where each bit is represented. With a bitrate of 44100 Hz each cell is 147 samples long
		k = (k+1)%samplesPerCell; 
		if (k == 0){			
			// Compare and decide
			markEnvOut = Filter_get(markEnv);
			spaceEnvOut = Filter_get(spaceEnv);			
			
			if (markEnvOut > spaceEnvOut && markEnvOut >= threshold){
				cell = MARK;
			}else if(spaceEnvOut > markEnvOut && spaceEnvOut >= threshold){
				cell = SPACE;
			} else{
				continue;
			}					

			// The FSM to decode the bitstream
			if (state == STOP && cell == SPACE){
				state = READ; 				// Got a startbit. Move to READ
			}else if (state == READ){				
				byte = cell << j | byte; 		// Build the byte
				j=(j+1)%8; 				// Count cells read
				if (j==0) state = RSTOP; 		// Move to RSTOP after reading 8 cells
			}else if (state == RSTOP){
				if (cell == MARK){					
					fprintf(bitstream,"%c",byte); 	// Got a stop bit. Append the byte
					byte = 0x00; 			// Reset the byte					
					state = STOP; 			// Finished reading the word. Move to STOP
				}else{
					printf("Error: Expected a stopbit (mark) but got a space!\n");
					state = STOP;
				}				
			}
		}		

		if (plot){
			// Write to the temp file as we go 
			fprintf(plotData, "%d %d %f %f %f %f %d\n", i, buffer[i], Filter_get(lpf), Filter_get(hpf), Filter_get(spaceEnv), Filter_get(markEnv), cell);
		}
	}
	
	fclose(bitstream);

	if (plot){
		fclose(plotData);
		FILE *gnuPlotPipe = popen("gnuplot -persistent", "w");
		fprintf(gnuPlotPipe, "%s\n", "plot 'plot.tmp' using 1:2 w l title 'raw', '' using 1:3 w l title 'lowpass', '' using 1:4 w l title 'highpass', '' using 1:5 w l title 'lowenv', '' using 1:6 w l title 'highenv', '' using 1:7 w l title 'bitstream'; pause mouse close");
		fclose(gnuPlotPipe);
	}

	return 0;
}

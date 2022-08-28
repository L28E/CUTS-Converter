#include "wavmeta.h"
#include "wavschema.h"
#include "modulation.h"

/*
 * Function:  writeCell
 * --------------------
 * Writes either a mark or space depending on passed parameters.
 *
 * The sine function gives a (sort of) PAM signal with a range (-1,1)
 * The PAM signal is then quantized, giving a PCM signal.
 * Lets use signed 16 bit PCM. Gives 65536 levels bewteen -32768 and 32767
 */
void writeCell(int samplesPerCell, double sampleFactor, double volumeFactor,
		double maxAmpl, FILE *fptr) {
	double pamSample;
	int16_t pcmSample;
	for (int t = 0; t < samplesPerCell; t++) {
		pamSample = sin(sampleFactor * (double) t);
		pcmSample = (int16_t) (volumeFactor * pamSample * maxAmpl);
		fwrite(&pcmSample, 2, 1, fptr);
	}
}

/*
 * Function:  writeFrame
 * --------------------
 * Writes a frame i.e. a startbit (a space), a byte, then a stopbit (a mark)
 * The byte is written to the tape in little endian.
 */
void writeFrame(int samplesPerCell, double markSampleFactor,
		double spaceSampleFactor, double volumeFactor, double maxAmpl,
		FILE *fptr, char byte) {
	uint8_t mask = 0x01; // 0b00000001 , a bit mask for the least significant bit

	// Write the startbit (Space)
	writeCell(samplesPerCell, spaceSampleFactor, volumeFactor, maxAmpl, fptr);

	// Write the byte (in little endian)
	for (int i = 0; i < 8; ++i) {
		if (mask << i & byte) {
			writeCell(samplesPerCell, markSampleFactor, volumeFactor, maxAmpl,
					fptr);
		} else {
			writeCell(samplesPerCell, spaceSampleFactor, volumeFactor, maxAmpl,
					fptr);
		}
	}

	// Write the stopbit(s)
	writeCell(samplesPerCell, markSampleFactor, volumeFactor, maxAmpl, fptr);
}

/*
 * Function:  modulate
 * --------------------
 * Converts an inputfile into a CUTS encoded WAV file
 *
 */
int modulate(char inputFile[],char outputFile[]) {	
	double pamSample;
	int16_t pcmSample;
	double maxAmpl = pow(2, bitDepth - 1) - 1;
	uint8_t *inputBuffer;
	long inputFilelen;

	WavMetadata meta;
	FILE *outputFilePtr, *inputFilePtr;

	// Open the file for reading and get its length in bytes	
	inputFilePtr = fopen(inputFile, "rb");
	if (inputFilePtr == NULL)
		return -1;
	fseek(inputFilePtr, 0, SEEK_END);
	inputFilelen = ftell(inputFilePtr);
	rewind(inputFilePtr);

	// Allocate memory for and read for the whole file.
	inputBuffer = (uint8_t*) malloc(inputFilelen * sizeof(uint8_t));
	fread(inputBuffer, inputFilelen, 1, inputFilePtr);
	fclose(inputFilePtr);

	// Open a file for writing.
	outputFilePtr = fopen(outputFile, "wb");
	if (outputFilePtr == NULL)
		return -1;

	double leadSize = 5 * 2 * sampleRate;
	/* 	- Each byte in the input file is encoded with 10 cells
			- Each cell has 147 samples
			- Each sample is 2 bytes
	*/  
	double dataSize = inputFilelen * 10 * samplesPerCell * 2; 

	// Populate the header and write it to the file.
	writeMeta(&meta, leadSize + dataSize, sampleRate, numChannels, bitDepth);
	fwrite(&meta, sizeof(meta), 1, outputFilePtr);

	// Start /w 5 seconds of mark.
	for (int t = 0; t < 5 * sampleRate; t++) {
		pamSample = sin(markSampleFactor * (double) t);
		pcmSample = (int16_t) (volumeFactor * pamSample * maxAmpl);
		fwrite(&pcmSample, 2, 1, outputFilePtr);
	}

	// Write the data.
	for (int i = 0; i < inputFilelen; i++) {
		writeFrame(samplesPerCell, markSampleFactor, spaceSampleFactor,
				volumeFactor, maxAmpl, outputFilePtr, inputBuffer[i]);
	}

	// Close the wav file.
	fclose(outputFilePtr);
	return 0;

}
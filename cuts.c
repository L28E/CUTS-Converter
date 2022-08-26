#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "filter.h"
#include "wavmeta.h"

const int sampleRate = 44100;
const int bitDepth = 16;
const double volumeFactor = 0.1;
const int numChannels = 1;
SampleFilter *lpf;
//Lets stick to the 300 Baud parameters for now
const int markFrequency = 2400;
const int spaceFrequency = 1200;
const double markSampleFactor = 2 * M_PI * markFrequency / sampleRate;
const double spaceSampleFactor = 2 * M_PI * spaceFrequency / sampleRate;
const int samplesPerCell = 147; // numcycles / freq * samplerate

/*
 * Function:  writeCell
 * --------------------
 * Writes either a mark or space depending on passed parameters.
 *
 * The sine function gives a PAM signal with a range (-1,1)
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

int modulate() {
	double pamSample;
	int16_t pcmSample;
	double maxAmpl = pow(2, bitDepth - 1) - 1;
	uint8_t *inputBuffer;
	long inputFilelen;

	WavMetadata meta;
	FILE *outputFilePtr, *inputFilePtr;

	// Open the file for reading and get its length in bytes
	// TODO: paramterize the input file
	inputFilePtr = fopen("test.txt", "rb");
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
	outputFilePtr = fopen("waveform.wav", "wb");
	if (outputFilePtr == NULL)
		return -1;

	double leadSize = 5 * 2 * sampleRate;
	double dataSize = inputFilelen * 8 * samplesPerCell * 2;

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

int demodulate() {
	FILE *inputFilePtr;
	uint32_t dataSize;
	int16_t *buffer;

	// Assuming we've recorded to a cassette, and then digitally recorded the casette back to a wav.
	// Open the wav for reading.
	inputFilePtr = fopen("waveform.wav", "rb");
	if (inputFilePtr == NULL) {
		return -1;
	}

	// For now, I'll assume all the parameters of this .wav are the same as the one from wavWrite.c
	// In that case I just need to get the datasize.
	// Note that because this is 16 bit audio i.e. 2 bytes per sample, the number of samples is half the datasize.
	fseek(inputFilePtr, 40, SEEK_SET);
	fread(&dataSize, 4, 1, inputFilePtr);
	printf("Data size in bytes: %d\n", dataSize);

	// Put all the wav data in a buffer
	buffer = (int16_t*) malloc(dataSize * sizeof(int16_t));
	fread(buffer, dataSize, 1, inputFilePtr);
	fclose(inputFilePtr);

	/* I don't really know what to do after this.
	 Demodulation will probably need to be de-coherent, because the casette will introduce a phase difference
	 Maybe do two simoultaneous convolutions to filter each frequency,
	 Then each goes into an envelope detector,
	 Then both go into one comparator which decides what the bit is at Tb (the period for one bit).
	 
	 The challenge is deciding Tb and keeping it synchronized.
	 */

	lpf = malloc(sizeof(SampleFilter) + sizeof(impulseReponse));
	SampleFilter_init(lpf);

	FILE *plotData = fopen("data.tmp", "w");

	for (int i = 0; i < dataSize / 2; i++) {
		SampleFilter_put(lpf, buffer[i]);
		fprintf(plotData, "%d %d %f\n", i, buffer[i], SampleFilter_get(lpf));
	}
	fclose(plotData);

//	FILE *gnuPlotPipe = popen("gnuplot -persistent", "w");
//	fprintf(gnuPlotPipe, "%s\n", "plot 'data.tmp' w l");
//	fclose(gnuPlotPipe);

	return 0;
}

int main(int argc, char *argv[]) {

	// TODO: make a nice cli
	if (strcmp(argv[1], "mod") == 0) {
		return modulate();
	} else if (strcmp(argv[1], "demod") == 0) {
		return demodulate();
	} else {
		return -1;
	}

}

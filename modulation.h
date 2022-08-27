#ifndef MODULATION_H_
#define MODULATION_H_

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

static const int sampleRate = 44100;
static const int bitDepth = 16;
static const double volumeFactor = 0.1;
static const int numChannels = 1;
//Lets stick to the 300 Baud parameters for now
static const int markFrequency = 2400;
static const int spaceFrequency = 1200;
static const double markSampleFactor = 2 * M_PI * markFrequency / sampleRate;
static const double spaceSampleFactor = 2 * M_PI * spaceFrequency / sampleRate;
static const int samplesPerCell = 147; // numcycles / freq * samplerate

void writeCell(int samplesPerCell, double sampleFactor, double volumeFactor,
		double maxAmpl, FILE *fptr); 
void writeFrame(int samplesPerCell, double markSampleFactor,
        double spaceSampleFactor, double volumeFactor, double maxAmpl,
        FILE *fptr, char byte);
int modulate(char inputFile[],char outputFile[]);

#endif
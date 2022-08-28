#ifndef MODULATION_H_
#define MODULATION_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void writeCell(int samplesPerCell, double sampleFactor, double volumeFactor,
		double maxAmpl, FILE *fptr); 
void writeFrame(int samplesPerCell, double markSampleFactor,
        double spaceSampleFactor, double volumeFactor, double maxAmpl,
        FILE *fptr, char byte);
int modulate(char inputFile[],char outputFile[]);

#endif
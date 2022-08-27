#include <string.h>

#include "modulation.h"
#include "demodulation.h"

int main(int argc, char *argv[]) {

	// TODO: make a nice cli
	if (strcmp(argv[1], "mod") == 0) {
		return modulate("test.txt","waveform.wav");
	} else if (strcmp(argv[1], "demod") == 0) {
		return demodulate("waveform.wav");
	} else {
		return -1;
	}

}

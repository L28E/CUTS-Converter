#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "modulation.h"
#include "demodulation.h"

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

typedef enum Mode
{
	MODULATE,
	DEMODULATE
} Mode;

int main(int argc, char **argv)
{

	int opt;
	bool m = false;
	bool i = false;
	bool o = false;
	bool p = false;
	char *inputFile;
	char *outputFile;
	Mode mode;

	while ((opt = getopt(argc, argv, "m:i:o:p")) != -1)	{
		switch (opt) {
		case 'm':
			m = true;
			if (strcmp(optarg, "enc") == 0) {
				mode = MODULATE;
			}
			else if (strcmp(optarg, "dec") == 0) {
				mode = DEMODULATE;
			}
			else {
				printf("Invalid mode provided!\nUsage: %s -m {enc|dec} -i input_file -o output_file [-p]\n", argv[0]);
				exit(EXIT_FAILURE);
			}
			break;
		case 'i':
			i = true;
			inputFile = malloc(sizeof(optarg));
			strcpy(inputFile, optarg);
			break;
		case 'o':
			o = true;
			outputFile = malloc(sizeof(optarg));
			strcpy(outputFile, optarg);
			break;
		case 'p':
			p = true;
			break;
		default: /* '?' */
			printf("Usage: %s -m {enc|dec} -i input_file -o output_file [-p]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (m && i && o)
	{
		// All arguments provided
		if (mode == MODULATE)
		{
			return modulate(inputFile, outputFile);
		}
		else
		{
			return demodulate(inputFile, outputFile, p);
		}
		free(inputFile);
		free(outputFile);
	}
	else
	{
		printf("Not enough arguments provided!\nUsage: %s -m {enc|dec} -i input_file -o output_file [-p]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

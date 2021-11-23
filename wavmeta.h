#include <stdint.h>
#include <string.h>

typedef struct {
	// The RIFF chunk
	uint8_t chunkID[4];
	uint32_t chunkSize;
	uint8_t format[4];
	// The format subchunk
	uint8_t subchunk1ID[4];
	uint32_t subchunk1Size;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	// The data subchunk
	uint8_t subchunk2ID[4];
	uint32_t subchunk2Size;
} WavMetadata;

void writeMeta(WavMetadata *meta, int totalSize, int sampleRate,
		int numChannels, int bitDepth) {
	memcpy(meta->chunkID, "RIFF", 4);
	meta->chunkSize = totalSize + 36;
	memcpy(meta->format, "WAVE", 4);
	memcpy(meta->subchunk1ID, "fmt ", 4);
	meta->subchunk1Size = 16;
	meta->audioFormat = 1;
	meta->numChannels = numChannels;
	meta->sampleRate = sampleRate;
	meta->byteRate = sampleRate * numChannels * bitDepth / 8;
	meta->blockAlign = numChannels * bitDepth / 8;
	meta->bitsPerSample = bitDepth;
	memcpy(meta->subchunk2ID, "data", 4);
	meta->subchunk2Size = totalSize;
}

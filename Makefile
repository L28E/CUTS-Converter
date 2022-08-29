CC ?= gcc

all: cuts

cuts: cuts.o filter.o modulation.o demodulation.o
	$(CC) -o cuts cuts.o filter.o modulation.o demodulation.o -lm

cuts.o: cuts.c wavmeta.h wavschema.h filter.h modulation.h demodulation.h
	$(CC) -c cuts.c
	
filter.o: filter.c filter.h
	$(CC) -c filter.c 

modulation.o: modulation.c modulation.h
	$(CC) -c modulation.c

demodulation.o: demodulation.c demodulation.h
	$(CC) -c demodulation.c

clean:
	rm -f *.o

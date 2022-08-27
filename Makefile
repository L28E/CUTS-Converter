all: cuts

cuts: cuts.o filter.o modulation.o demodulation.o
	gcc -o cuts cuts.o filter.o modulation.o demodulation.o -lm

cuts.o: cuts.c wavmeta.h filter.h modulation.h demodulation.h
	gcc -c cuts.c
	
filter.o: filter.c filter.h
	gcc -c filter.c 

modulation.o: modulation.c modulation.h
	gcc -c modulation.c

demodulation.o: demodulation.c demodulation.h
	gcc -c demodulation.c

clean:
	rm -f *.o

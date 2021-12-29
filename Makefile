all: cuts

cuts: cuts.o filter.o 
	gcc -o cuts cuts.o filter.o -lm

cuts.o: cuts.c wavmeta.h filter.h
	gcc -c cuts.c
	
filter.o: filter.c filter.h
	gcc -c filter.c 

clean:
	rm -f *.o cuts

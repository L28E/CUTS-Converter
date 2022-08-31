# CUTS-Converter
A program to convert arbitrary files to or from CUTS encoding. The intent is to store and retrieve (small) files on an audio cassette.

## About CUTS

|Cell|300 Bd|1200 Bd|2400 Bd (MSX)|2400 Bd (Quick CUTS)|
|---|---|---|---|---|
|Mark (1)|8 cycles @ 2400 Hz|2 cycles @ 2400 Hz|2 cycles @ 4800 Hz|1 cycle @ 2400 Hz|
|Space (0)|4 cycles @ 1200 Hz|1 cycle @ 1200 Hz|1 cycle @ 2400 Hz|0.5 cycles @ 1200 Hz|

- Per the original 300 Bd standard, bits are Frequency-Shift Keyed into "cells", which can be either a "mark" or a "space" 
    - A mark represents a 1 and consists of eight cycles at a frequency of 2400 Hz
    - A space represents a 0 and consists of four cycles at a frequency of 1200 Hz 
- A "dataframe" or just "frame" consists of a startbit (a space), a byte/word, then a stopbit (a mark) 
    - A word is recorded in little endian order, i.e. little end first

At the start of the recording there is 5 seconds of mark for synchronization, then frames. There can be an arbitrary number of marks between frames. 

## Usage
`./cuts -m {enc|dec} -i input_file -o output_file [-p]`

- To encode, use `enc` after the `-m` option. To decode, instead use `dec` after the `-m` option
- To optionally plot the decoding results, use the `-p` option
- For Windows friends the usage is more or less the same, but first run `cmd`, `cd` into the directory with the executable and then run the program, using `cuts_x86.exe` instead of `cuts`

## Building
- `make` to compile for Linux
- `make CC=i686-w64-mingw32-gcc` to cross-compile for Windows``

## Dependencies
- gnuplot is necessary to see the aforementioned plot, but the program can be run normally without it

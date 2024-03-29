CC = g++
CFLAGS = -g -Wall #-pedantic-errors #-Werror
LIBS = -lz -L. -lgzstream

#all:
#	echo ' what to run'


run_interps: interps
	./interps NZ/data
interps: interps.cpp parserra.* urltable.* fwdindex.* record.* config.h
	$(CC) $(CFLAGS) $(LIBS) -o interps $^ 

run_merger: merger
	./merger srcpath tmpath destpath
merger: merger.cpp 
	$(CC) $(CFLAGS) $(LIBS) -o merger $^

run_formatter: formatter
	rm -rf destpath/i2list destpath/lexicon.gz
	./formatter destpath 
formatter: formatter.cpp record.*
	$(CC) $(CFLAGS) $(LIBS) -o formatter $^


clean:
	rm -f interps merger formatter 
	rm -rf gmon.out *.dSYM
cleanb:
	rm -rf urltable/* fwdbarrel/*
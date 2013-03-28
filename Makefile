CC = g++
CFLAGS =  -O2 -g -Wall #-pedantic-errors #-Werror
LIBS = -lz -L. -lgzstream

run_formatter: formatter
	rm -rf destpath/i2list destpath/lexicon.gz
	./formatter destpath 
formatter: formatter.cpp record.*
	$(CC) $(CFLAGS) $(LIBS) -o formatter $^

run_merger: merger
	rm -rf tmpath/temp* destpath/temp*
	./merger srcpath tmpath destpath
merger: merger.cpp 
	$(CC) $(CFLAGS) $(LIBS) -o merger $^

run_interps: interps
	./interps NZ/data
interps: interps.cpp parserra.* urltable.* fwdindex.* record.* config.h
	rm -rf urltable/* fwdbarrel/*
	$(CC) $(CFLAGS) $(LIBS) -o interps $^ 

clean:
	rm -rf urltable/* fwdbarrel/*
	rm -f interps merger formatter 
	rm -rf gmon.out *.dSYM
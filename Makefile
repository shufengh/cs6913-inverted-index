CC = g++
CFLAGS = -g -Wall
LIBS = -lz

run: test
	./test
test: main.cpp parser.c urltable.cpp
	$(CC) $(CFLAGS) $(LIBS) -o test $^ 
parser.o: 
	make -C parser
clean:
	rm -f *.o parser/*.o
	rm -f test parser/test
	rm -rf gmon.out test.dSYM
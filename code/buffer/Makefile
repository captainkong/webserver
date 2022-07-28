CC = g++
CFLAGS = -Wall -g
LIBS = 

PROGS =	test

all:	${PROGS}

test: test.cpp buffer.o
		${CC} ${CFLAGS} -o $@ $^ ${LIBS}

buffer.o: buffer.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}
clean:
		rm -f ${PROGS} *.o

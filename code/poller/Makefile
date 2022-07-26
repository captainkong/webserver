CC = g++
CFLAGS = -Wall
LIBS =  -lpthread

PROGS =	test

all:	${PROGS}

test: test.cpp epoller.o
		${CC} ${CFLAGS} -o $@ $^ ${LIBS}

epoller.o: epoller.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}
clean:
		rm -f ${PROGS} *.o

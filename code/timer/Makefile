CC = g++
CFLAGS = -Wall
LIBS =  -lpthread

PROGS =	test

all:	${PROGS}

test: test.cpp heaptimer.o 
		${CC} ${CFLAGS} -o $@ $^  ${LIBS}

heaptimer.o: heaptimer.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}
clean:
		rm -f ${PROGS} *.o

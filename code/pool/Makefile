CC = g++
CFLAGS = -Wall -std=c++11
LIBS =   -lmysqlclient -pthread

PROGS =	test

all:	${PROGS}

test:sqlconnpool.o test.cpp
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

sqlconnpool.o:sqlconnpool.cpp
	${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}
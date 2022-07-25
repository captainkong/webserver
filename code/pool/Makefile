CC = g++
CFLAGS = -Wall -std=c++11
LIBS =   -lmysqlclient -pthread

PROGS =	test-scp test-tp

all:	${PROGS}

test-scp:sqlconnpool.o test-scp.cpp
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

test-tp:threadpool.o test-tp.cpp
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

sqlconnpool.o:sqlconnpool.cpp
	${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

threadpool.o:threadpool.cpp
	${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

clean:
	rm -f ${PROGS} *.o
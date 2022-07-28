CC = g++
CFLAGS = -Wall
LIBS =  -lpthread

PROGS =	server

all:	${PROGS}

server: test.cpp echoserver.o ../buffer/buffer.o ../poller/epoller.o ../pool/threadpool.o
		${CC} ${CFLAGS} -o $@ $^  ${LIBS}

echoserver.o: echoserver.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../buffer/buffer.o: ../buffer/buffer.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../poller/epoller.o: ../poller/epoller.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../pool/threadpool.o: ../pool/threadpool.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

clean:
		rm -f ${PROGS} *.o

CC = g++
CFLAGS = -Wall -std=c++11
LIBS =  -lpthread

PROGS =	webserver

all:	${PROGS}

echoserver: test.cpp echoserver.o ../buffer/buffer.o ../poller/epoller.o ../pool/threadpool.o
		${CC} ${CFLAGS} -o $@ $^  ${LIBS}

webserver: server.cpp webserver.o ../buffer/buffer.o ../poller/epoller.o ../pool/threadpool.o ../pool/sqlconnpool.o ../http/httprequest.o ../http/httpresponse.o ../http/httpconnect.o 
		${CC} ${CFLAGS} -o $@ $^  ${LIBS} -lmysqlclient

echoserver.o: echoserver.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

webserver.o: webserver.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../http/httpconnect.o: ../http/httpconnect.cpp 
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../http/httprequest.o: ../http/httprequest.cpp 
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../http/httpresponse.o: ../http/httpresponse.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../buffer/buffer.o: ../buffer/buffer.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../poller/epoller.o: ../poller/epoller.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../pool/threadpool.o: ../pool/threadpool.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS}

../pool/sqlconnpool.o: ../pool/sqlconnpool.cpp
		${CC} ${CFLAGS} -c -o $@ $^ ${LIBS} -lmysqlclient

clean:
		rm -f ${PROGS} *.o

#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include "../pool/threadpool.h"
#include "../buffer/buffer.h"
#include "../poller/epoller.h"
#include <unordered_map>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <atomic>
#include <fcntl.h>
#include <iostream>
#include <functional>

using std::cout;
using std::endl;

struct client_t
{
    int port;
    char ip[16];
    Buffer readBuffer;
    client_t(int _port, const char *_ip) : port(_port)
    {
        strncpy(ip, _ip, 16);
    };
};

class EchoServer
{
private:
    EPoller epoll_;
    ThreadPool pool_;
    std::unordered_map<int, client_t *> users_;
    int listenFd_;
    std::atomic<bool> isClose_;

    void acceptNewClient();
    void readFromClient(int index);

public:
    EchoServer(int thread_size, int port_number, int max_user_count);
    ~EchoServer();
    void start();
};

#endif // ECHOSERVER_H
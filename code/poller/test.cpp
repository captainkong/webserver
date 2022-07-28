#include "epoller.h"
#include "../buffer/buffer.h"
#include "../pool/threadpool.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <unordered_map>

using namespace std;

#define MAX_USER 1024

struct conn_t
{
    int port;
    char ip[16];
    Buffer readBuffer, writeBuffer;
    conn_t(int _port, const char *_ip) : port(_port)
    {
        strncpy(ip, _ip, 16);
    };
};

EPoller gEpoll_(MAX_USER);
unordered_map<int, conn_t *> gMap;

void readFromClient(conn_t *client, int index)
{

    cout << "readFromClient 线程id:" << this_thread::get_id() << endl;
    // 处理客户端消息
    int clientFd = gEpoll_.getFd(index);
    int err;
    while (true)
    {
        int n = client->readBuffer.readFd(clientFd, &err);
        cout << "n=" << n << ", err=" << err << ", readable=" << client->readBuffer.readableBytes() << endl;
        if (n < 1)
        {
            if (err == EAGAIN)
            {
                break;
            }
            // 出现错误或者对方主动关闭 下树
            conn_t *tem = gMap[clientFd];
            printf("Closed: %s:%d\n", tem->ip, tem->port);
            delete tem;
            gMap.erase(clientFd);
            close(clientFd);
            gEpoll_.delFd(clientFd);
            return;
        }
    }

    string str = client->readBuffer.retrieveAllToString();
    for (int j = 0; j < static_cast<int>(str.size()); ++j)
    {
        if (str[j] >= 'a' && str[j] <= 'z')
        {
            str[j] -= 'a' - 'A';
        }
    }
    send(clientFd, str.data(), str.size(), 0);
    if (str[str.size() - 1] == '\n')
        str[str.size() - 1] = '\0';
    cout << "服务端转发:" << client->ip << ":" << client->port << " len=" << str.size() << endl;
}

// 实现一个echo服务器
int main(int argc, char const *argv[])
{
    int lfd, cfd, ret;
    struct sockaddr_in addr;
    // char buffer[1024];
    ThreadPool pool(8);

    // 创建套接字
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(lfd != -1);

    // 设置端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, (void *)&opt, sizeof(opt));

    // 绑定
    addr.sin_port = htons(8010);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(0);
    socklen_t len = sizeof(addr);
    ret = bind(lfd, (sockaddr *)&addr, len);
    assert(ret != -1);

    // 监听
    ret = listen(lfd, MAX_USER);
    gEpoll_.addFd(lfd, EPOLLIN);

    while (1)
    {
        int ready = gEpoll_.wait();
        // cout << "read:" << read << endl;
        for (int i = 0; i < ready; ++i)
        {
            if (gEpoll_.getFd(i) == lfd && gEpoll_.getEvent(i) & EPOLLIN)
            {
                // 新的连接到来
                struct sockaddr_in cliAddr;
                socklen_t len = sizeof(cliAddr);
                cfd = accept(lfd, (struct sockaddr *)&cliAddr, &len);
                if (cfd == -1)
                {
                    perror("连接建立失败!");
                }

                // 将cfd设置为非阻塞模式
                int flags = fcntl(cfd, F_GETFL);
                flags |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flags);

                char ip[16];
                inet_ntop(AF_INET, &cliAddr.sin_addr, ip, sizeof(ip));
                conn_t *newConn = new conn_t(ntohs(cliAddr.sin_port), ip);
                gMap[cfd] = newConn;
                printf("New Connect: %s:%d\n", newConn->ip, newConn->port);
                // 将新的连接添加到红黑树中
                ret = gEpoll_.addFd(cfd, EPOLLIN | EPOLLET);
                assert(ret);
            }
            else if (gEpoll_.getFd(i) != lfd && gEpoll_.getEvent(i) & EPOLLIN)
            {
                // 处理客户端消息
                int clientFd = gEpoll_.getFd(i);
                pool.addTask(bind(readFromClient, gMap[clientFd], i));
            }
        }
    }

    return 0;
}

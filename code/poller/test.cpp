#include "epoller.h"
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
    conn_t(int _port, const char *_ip) : port(_port)
    {
        strncpy(ip, _ip, 16);
    };
};

// 实现一个echo服务器
int main(int argc, char const *argv[])
{
    int lfd, cfd, ret;
    struct sockaddr_in addr;
    EPoller epoll_(MAX_USER);
    char buffer[1024];
    unordered_map<int, conn_t *> d;

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
    epoll_.addFd(lfd, EPOLLIN);

    while (1)
    {
        int ready = epoll_.wait();
        // cout << "read:" << read << endl;
        for (int i = 0; i < ready; ++i)
        {
            if (epoll_.getFd(i) == lfd && epoll_.getEvent(i) & EPOLLIN)
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
                d[cfd] = newConn;
                printf("New Connect: %s:%d\n", newConn->ip, newConn->port);
                // 将新的连接添加到红黑树中
                ret = epoll_.addFd(cfd, EPOLLIN);
                assert(ret);
            }
            else if (epoll_.getFd(i) != lfd && epoll_.getEvent(i) & EPOLLIN)
            {
                // 处理客户端消息
                int clientFd = epoll_.getFd(i);
                memset(buffer, 0, sizeof(buffer));
                int n = read(clientFd, buffer, sizeof(buffer) - 1);
                if (n < 1)
                {
                    // 出现错误或者对方主动关闭 下树
                    conn_t *tem = d[clientFd];
                    printf("Closed: %s:%d\n", tem->ip, tem->port);
                    delete tem;
                    d.erase(clientFd);
                    close(clientFd);
                    epoll_.delFd(clientFd);

                    continue;
                }

                buffer[strlen(buffer)] = '\0';
                for (int j = 0; j < static_cast<int>(strlen(buffer)); ++j)
                {
                    if (buffer[j] >= 'a' && buffer[j] <= 'z')
                    {
                        buffer[j] -= 'a' - 'A';
                    }
                }
                send(clientFd, buffer, strlen(buffer), 0);
                if (buffer[strlen(buffer) - 1] == '\n')
                    buffer[strlen(buffer) - 1] = '\0';
                printf("服务端转发:%s\n", buffer);
            }
        }
    }

    return 0;
}

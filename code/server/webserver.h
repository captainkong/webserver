#ifndef WEBSERVER_H
#define WEBSERVER_H
/**
 * Web Server 跟Echo Server的异同点:
 * 相同之处:都要从客户端读数据,写数据(相应请求)
 * 不同之处:web server 接收到的客户端请求不再是简单的字符串流,而是结构化的
 * http请求,服务端需要分析http请求的各个字段,判断请求的类型,合法性,资源路径
 * 再根据http协议的结构将处理结果返回给客户端
 *
 * echoserver 的tcp连接 对应到webserver 为http连接
 */

#include "../pool/threadpool.h"
#include "../pool/sqlconnpool.h"
#include "../buffer/buffer.h"
#include "../poller/epoller.h"
#include "../http/httpconnect.h"
#include <unordered_map>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <atomic>
#include <fcntl.h>
#include <iostream>
#include <functional>

/**
 * 可能的内存泄露问题 HTTPConnect *
 *
 */

class WebServer
{
private:
    EPoller epoll_;
    ThreadPool pool_;
    std::atomic<bool> isClose_;
    std::unordered_map<int, HttpConnect *> users_;
    int listenFd_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    // lfd epollin 触发
    void acceptNewClient();
    // cfd epollin 触发
    void dealClientRead(int cfd);
    void onProcess(HttpConnect *client);
    // cfd epollout 触发
    void dealClientWrite(int cfd);

    // 关闭客户端连接
    void closeConnect(HttpConnect *con);

public:
    WebServer(int thread_size, int port_number, int max_user_count);
    ~WebServer();
    // 打开服务器,开启事件循环
    void start();
};

#endif // WEBSERVER
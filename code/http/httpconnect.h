#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"
#include <arpa/inet.h>
#include <sys/types.h>

/**
 * HttpConnect 对应到echoServer的结构体client_t
 * 它应该包含连接的所有信息:
 * 需要应用层的读写缓冲区
 * 由于读写的逻辑远复杂于echoServer,需要单独封装成类
 *
 */

class HttpConnect
{
private:
    int port_;
    char ip_[16];
    int fd_;
    struct iovec iov_[2];
    size_t iovCnt_;
    Buffer readBuffer_;
    Buffer writeBuffer_;
    HttpRequest request_;
    HttpResponse response_;

public:
    HttpConnect(int cfd, struct sockaddr_in &clientAddr);
    ~HttpConnect();
    size_t readFromClient(int *err);
    size_t sendToClient(int *err);
    int getFd() const;
    const char *getIP() const;
    int getPort() const;
    bool praseRequest();

    static const char *wwwRoot;
};

#endif // HTTP_CONNECT_H
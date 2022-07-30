#include "httpconnect.h"

HttpConnect::HttpConnect(int cfd, struct sockaddr_in &clientAddr) : fd_(cfd)
{
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip_, sizeof(ip_));
    port_ = ntohs(clientAddr.sin_port);
}

HttpConnect::~HttpConnect()
{
}

size_t HttpConnect::readFromClient(int *err)
{
    size_t total = 0;
    // ET模式
    while (true)
    {
        size_t len = readBuffer_.readFd(fd_, err);
        total += len;
        std::cout << "len:" << len << ", err=" << *err << std::endl;
        if (len <= 0 || *err == EAGAIN)
            break;
    }
    return total;
}

size_t HttpConnect::sendToClient(int *err)
{
    return 0;
}

bool HttpConnect::praseRequest()
{
    assert(readBuffer_.readableBytes() > 0);
    return request_.prase(readBuffer_);
}

int HttpConnect::getFd() const
{
    return fd_;
}

const char *HttpConnect::getIP() const
{
    return ip_;
}

int HttpConnect::getPort() const
{
    return port_;
}
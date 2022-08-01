#include "httpconnect.h"

using std::cout;
using std::endl;
#include <sys/time.h>

const char *HttpConnect::wwwRoot;

HttpConnect::HttpConnect(int cfd, struct sockaddr_in &clientAddr) : fd_(cfd), response_(wwwRoot)
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
    // 使用分散写
    size_t len = -1;
    while (true)
    {
        len = writev(fd_, iov_, iovCnt_);
        cout << "len:" << len << ",err=" << errno << endl;
        if (len <= 0)
        {
            *err = errno;
            break;
        }
        if (len < iov_[0].iov_len)
        {
            // 缓冲区没有读完,更新缓冲区
            writeBuffer_.retrieve(len);
            iov_[0].iov_base = const_cast<char *>(writeBuffer_.beginRead());
            iov_[0].iov_len = writeBuffer_.readableBytes();
        }
        else
        {
            writeBuffer_.retrieveAll();
            if (iovCnt_ == 2)
            {
                if (len == iov_[0].iov_len + iov_[1].iov_len)
                {
                    // 读干净,可以退出
                    break;
                }
                size_t spend = len - iov_[0].iov_len;
                iov_[1].iov_base = (uint8_t *)iov_[1].iov_base + spend;
                iov_[1].iov_len -= spend;
                iov_[0].iov_len = 0;
            }
        }
    }

    return 0;
}

bool HttpConnect::praseRequest()
{
    // timeval startTime, endTime;
    // gettimeofday(&startTime, NULL);

    assert(readBuffer_.readableBytes() > 0);
    bool ret = request_.prase(readBuffer_);
    string path = request_.getPath();

    // gettimeofday(&endTime, NULL);
    // int timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
    // cout << "第1阶段用时:" << timeUsed << "us" << endl;
    // gettimeofday(&startTime, NULL);

    // cout << "HttpConnect::praseRequest path=" << path << endl;
    if (ret)
    {
        response_.response(&writeBuffer_, path, true, 200);
    }
    else
    {
        response_.response(&writeBuffer_, path, true, 400);
    }

    // gettimeofday(&endTime, NULL);
    // timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
    // cout << "第2阶段用时:" << timeUsed << "us" << endl;
    // gettimeofday(&startTime, NULL);

    iov_[0].iov_base = const_cast<char *>(writeBuffer_.beginRead());
    iov_[0].iov_len = writeBuffer_.readableBytes();
    iovCnt_ = 1;

    if (response_.file() != nullptr && response_.fileSize() > 0)
    {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileSize();
        iovCnt_ = 2;
    }

    // gettimeofday(&endTime, NULL);
    // timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
    // cout << "第3阶段用时:" << timeUsed << "us" << endl;
    return true;
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
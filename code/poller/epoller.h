#ifndef EPOLLER_H
#define EPOLLER_H

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>

class EPoller
{
private:
    int epollFd_;
    std::vector<struct epoll_event> events_;

    EPoller() = delete;
    EPoller(const EPoller &) = delete;
    void operator=(const EPoller &) = delete;

public:
    explicit EPoller(int maxEventSize);

    // 添加监听节点
    bool addFd(int fd, u_int32_t evnets);

    // 删除监听节点
    bool delFd(int fd);

    // 修改节点
    bool modFd(int fd, u_int32_t evnets);

    // 等待epollFd_实例的事件返回,返回值为事件数量,不加参数会无限等待
    int wait(int timeout_ms = -1);

    // 根据下标获取事件
    u_int32_t getEvent(int index);

    // 根据下标获取文件描述符
    int getFd(int index);

    ~EPoller();
};

#endif // EPOLLER_H
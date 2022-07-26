#include "epoller_.h"

EPoller::EPoller(int maxEventSize) : epollFd_(epoll_create(maxEventSize)), events_(maxEventSize)
{
    assert(epollFd_ > 0 && events_.size() > 0);
}

EPoller::~EPoller()
{
    close(epollFd_);
}

bool EPoller::addFd(int fd, u_int32_t evnets)
{
    assert(fd > 0);
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = evnets;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

bool EPoller::delFd(int fd)
{
    assert(fd > 0);
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

bool EPoller::modFd(int fd, u_int32_t evnets)
{
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = evnets;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

int EPoller::wait(int timeout_ms)
{
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeout_ms);
}

u_int32_t EPoller::getEvent(int index)
{
    return events_[index].events;
}

int EPoller::getFd(int index)
{
    return events_[index].data.fd;
}
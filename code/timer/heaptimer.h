#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <functional>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <iostream>

typedef std::function<void()> timeOutCallBack;
typedef std::chrono::high_resolution_clock clocks;
typedef std::chrono::microseconds MS;
typedef clocks::time_point timeStamp;

struct timerNode
{
    size_t id; // 使用fd当做fd,方便查询
    timeStamp expire;
    timeOutCallBack cb;
    bool operator<(const timerNode &tn) const
    {
        return expire < tn.expire;
    }
};

class HeapTimer
{
private:
    std::vector<timerNode> heap_;
    std::unordered_map<size_t, size_t> ref_; // id(fd) -> idx

    // 堆元素上移
    void adjUp_(size_t idx);
    // 堆元素下移
    bool adjDown_(size_t idx, size_t lastIndex);
    // 删除堆元素
    void del_(size_t idx);
    // 交换堆元素
    void swap_(size_t left, size_t right);

public:
    HeapTimer();
    ~HeapTimer();
    void add(size_t id, size_t msTimeOut, timeOutCallBack _cb);
    void update(size_t id, size_t msTimeOut);
    // 处理过期定时器
    void tick();
    // 获取下次过期时间,同时执行已过期定时器的回调函数
    int getNextExpireTime();

    void display();
};

#endif
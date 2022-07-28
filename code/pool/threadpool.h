#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <assert.h>
#include <iostream>
#include <sys/time.h>
#include <time.h>

using std::function;

// 队列中的任务函数表现为无参数,可以使用bind添加参数
typedef function<void()> job;

class ThreadPool
{
private:
    std::queue<job> task_queue_;
    std::mutex mut_;
    std::condition_variable cond_;
    static void initTask(ThreadPool *_pool);
    bool isClose;

public:
    explicit ThreadPool(int threadCount);
    ~ThreadPool();
    void addTask(job &&task);
    void shutdown();
};

#endif // THREADPOOL_H
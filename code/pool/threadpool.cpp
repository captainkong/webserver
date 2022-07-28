#include "threadpool.h"

using std::cout;
using std::endl;

ThreadPool::ThreadPool(int threadCount) : isClose(false)
{
    assert(threadCount > 0);
    for (int i = 0; i < threadCount; ++i)
    {
        std::thread(initTask, this).detach();
    }
}

void ThreadPool::initTask(ThreadPool *_pool)
{
    // std::cout << "线程id:" << std::this_thread::get_id() << std::endl;
    ThreadPool *pool = _pool;
    std::unique_lock<std::mutex> locker(pool->mut_);
    timeval startTime, endTime;

    while (true)
    {
        // 首先判断是否退出线程
        if (pool->isClose)
            break;
        if (pool->task_queue_.size())
        {
            auto task = std::move(pool->task_queue_.front());
            pool->task_queue_.pop();

            // 让出锁,让其它抢锁拿任务
            locker.unlock();

            gettimeofday(&startTime, NULL);
            task();
            gettimeofday(&endTime, NULL);
            int timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
            cout << "线程执行用时:" << timeUsed << "us" << endl;

            // 执行完任务,继续抢锁 拿任务
            locker.lock();
        }
        else
        {
            pool->cond_.wait(locker);
        }
    }
    // std::cout << "线程id:" << std::this_thread::get_id() << "结束" << std::endl;
}

void ThreadPool::addTask(job &&task)
{
    {
        std::lock_guard<std::mutex> locker(mut_);
        task_queue_.emplace(std::move(task));
    }
    cond_.notify_one();
}

ThreadPool::~ThreadPool()
{
    if (!isClose)
    {
        shutdown();
    }
}

void ThreadPool::shutdown()
{
    // 诱骗所有线程拿锁 然后结束线程
    {
        std::lock_guard<std::mutex> locker(mut_);
        isClose = true;
    }
    cond_.notify_all();
}
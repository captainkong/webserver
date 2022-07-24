#include "sqlconnpool.h"
#include <iostream>

using std::cout;
using std::endl;

void SqlConnPool::init(const char *host, int port, const char *user,
                       const char *pwd, const char *dbName, int capacity)
{
    assert(capacity > 0);
    capacity_ = size_ = capacity;
    // 初始化信号量
    sem_init(&sem_, 0, capacity);
    for (int i = 0; i < capacity; ++i)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        assert(sql);
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        assert(sql);
        sqlConnQue.push(sql);
    }
}

SqlConnPool *SqlConnPool::getInstance()
{
    // 使用静态变量实现单例模式
    // 在类外实现不能指定static
    static SqlConnPool pool;
    return &pool;
}

SqlConnPool::~SqlConnPool()
{
    destory();
}

MYSQL *SqlConnPool::getSqlConnect()
{
    MYSQL *sql = nullptr;
    // p一个资源
    sem_wait(&sem_);
    //获取互斥锁
    {
        std::lock_guard<std::mutex> locker(mux_);
        sql = sqlConnQue.front();
        sqlConnQue.pop();
    }
    return sql;
}

void SqlConnPool::freeSqlConnect(MYSQL *sql)
{
    //获取互斥锁
    {
        std::lock_guard<std::mutex> locker(mux_);
        sqlConnQue.push(sql);
    }
    // v一个资源
    sem_post(&sem_);
}

int SqlConnPool::getFreeCount()
{
    std::lock_guard<std::mutex> locker(mux_);
    return sqlConnQue.size();
}

void SqlConnPool::destory()
{
    // 只能保证空闲的连接被安全关闭
    std::lock_guard<std::mutex> locker(mux_);
    while (sqlConnQue.size())
    {
        MYSQL *sql = sqlConnQue.front();
        sqlConnQue.pop();
        mysql_close(sql);
    }
    mysql_library_end();
}

SqlRAII::SqlRAII(MYSQL **sql, SqlConnPool *pool)
{
    sql_ = *sql = pool->getSqlConnect();
    sqlPool_ = pool;
}

SqlRAII::~SqlRAII()
{
    assert(sql_);
    sqlPool_->freeSqlConnect(sql_);
}
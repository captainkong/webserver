#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <mutex>
#include <semaphore.h>
#include <queue>
#include <assert.h>

class SqlConnPool
{
private:
    SqlConnPool() : capacity_(0), size_(0){};
    ~SqlConnPool();

    int capacity_;
    int size_;
    std::queue<MYSQL *> sqlConnQue;
    std::mutex mux_;
    sem_t sem_;

public:
    // 采用单例模式,使用init完成初始化工作 SqlConnPool::Instance()->init
    void init(const char *host, int port, const char *user,
              const char *pwd, const char *dbName, int capacity);

    // 获取线程池实例
    static SqlConnPool *getInstance();

    // 获取一个连接
    MYSQL *getSqlConnect();

    // 将连接放回连接池
    void freeSqlConnect(MYSQL *);

    // 获取空闲资源数量
    int getFreeCount();

    // 销毁连接池
    void destroy();
};

// 封装一个RAII风格的sql对象管理类
class SqlRAII
{
private:
    SqlConnPool *sqlPool_;
    MYSQL *sql_;

public:
    // 构造时获取资源
    SqlRAII(MYSQL **sql, SqlConnPool *pool);

    // 析构时释放资源
    ~SqlRAII();
};

#endif // SQLCONNPOOL_H
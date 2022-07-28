#ifndef BUFFER_H
#define BUFFER_H

#include <atomic>
#include <string>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

using std::size_t;
using std::string;
using std::vector;

class Buffer
{
private:
    vector<char> buffer_;
    std::atomic<size_t> readIndex_;
    std::atomic<size_t> writeIndex_;

    // 确保存在len字节的可写空间
    void ensureWriteableSize(size_t len);
    // 开辟len的可写空间
    void makeWriteSize(size_t len);

public:
    explicit Buffer(int _capacity = 1024);
    ~Buffer();

    // 缓冲区可读字节数
    size_t readableBytes() const;
    // 缓冲区可写字节数
    size_t writeableBytes() const;
    // 前置空余字节数
    size_t preBytes();

    // 读指针
    const char *beginRead() const;
    // 写指针
    char *beginWrite();
    const char *beginWriteConst() const;

    // 缓冲区已经读入len字节数据,更新读指针
    void retrieve(size_t len);
    // 重置缓冲区
    void retrieveAll();
    // 将缓冲区的数据导出为string
    string retrieveAllToString();

    // 向缓冲区追加写入字符串
    void append(const string &str);
    void append(const char *src, size_t len);
    void append(const void *src, size_t len);
    void append(const Buffer &buff);

    // 从文件描述符fd中读数据到缓冲区
    size_t readFd(int fd, int *err);
    // 向文件描述符fd中写入缓冲区的数据
    size_t writeFd(int fd, int *err);
};

#endif
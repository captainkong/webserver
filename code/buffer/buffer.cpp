#include "buffer.h"
#include <iostream> // for debug

void Buffer::ensureWriteableSize(size_t len)
{
    if (writeableBytes() < len)
    {
        makeWriteSize(len);
    }
    assert(writeableBytes() >= len);
}

void Buffer::makeWriteSize(size_t len)
{
    size_t remain = preBytes() + writeableBytes();
    // 整体前移, 让出空闲空间
    std::copy(buffer_.begin() + readIndex_, buffer_.begin() + writeIndex_, buffer_.begin());
    if (remain < len)
    {
        int need = len - remain;
        buffer_.resize(buffer_.size() + need);
    }
}

Buffer::Buffer(int _capacity) : buffer_(_capacity), readIndex_(0), writeIndex_(0)
{
}

Buffer::~Buffer()
{
}

size_t Buffer::readableBytes() const
{
    return writeIndex_ - readIndex_;
}

size_t Buffer::writeableBytes() const
{
    return buffer_.size() - writeIndex_;
}

size_t Buffer::preBytes()
{
    return readIndex_;
}

const char *Buffer::beginRead() const
{
    return &*buffer_.begin() + readIndex_;
}

char *Buffer::beginWrite()
{
    return &*buffer_.begin() + writeIndex_;
}

const char *Buffer::beginWriteConst() const
{
    return &*buffer_.begin() + writeIndex_;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    readIndex_ += len;
}

void Buffer::retrieveAll()
{
    readIndex_ = writeIndex_ = 0;
}

string Buffer::retrieveAllToString()
{
    string str(beginRead(), readableBytes());
    retrieveAll();
    return str;
}

void Buffer::append(const string &str)
{
    append(str.data(), str.size());
}

void Buffer::append(const char *src, size_t len)
{
    assert(src);
    ensureWriteableSize(len);
    std::copy(src, src + len, beginWrite());
    writeIndex_ += len;
}

void Buffer::append(const void *src, size_t len)
{
    assert(src);
    append(static_cast<const char *>(src), len);
}

void Buffer::append(const Buffer &buff)
{
    append(buff.beginRead(), buff.readableBytes());
}

size_t Buffer::readFd(int fd, int *err)
{
    char tem[65535];
    struct iovec iov[2];
    iov[0].iov_base = const_cast<char *>(beginWriteConst());
    iov[0].iov_len = writeableBytes();
    iov[1].iov_base = tem;
    iov[1].iov_len = sizeof(tem);

    const size_t writeable = writeableBytes();
    ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        if (err)
        {
            *err = errno;
        }
    }
    else if (static_cast<size_t>(len) <= writeableBytes())
    {
        // 数据已经完全写到缓冲区,直接更改写索引即可
        writeIndex_ += len;
    }
    else
    {
        // 缓冲区已经写满,需要将tem中的数据append到buffer中
        writeIndex_ = buffer_.size();
        append(tem, len - writeable);
    }
    return static_cast<size_t>(len);
}

size_t Buffer::writeFd(int fd, int *err)
{
    size_t writeLen = readableBytes();
    ssize_t len = write(fd, beginRead(), writeLen);
    if (len < 0)
    {
        if (err)
        {
            *err = errno;
        }
    }
    else
    {
        readIndex_ += len;
    }
    return static_cast<size_t>(len);
}
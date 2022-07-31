#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "../buffer/buffer.h"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

using std::string;
using std::unordered_map;

class HttpResponse
{
private:
    static const std::unordered_map<string, string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
    string rootDir_;
    Buffer *pBuff_;

    string path_;
    int statusCode_;
    bool isKeepAlive_;

    struct stat fileStat_;
    char *mmFile_;

    void makeStatusLine();
    void makeHeader();
    void makeBody();

    // 根据拓展名判断文件的类型
    string getFileType() const;
    // 释放 mmap
    void unmapFile();

public:
    HttpResponse(string path);
    ~HttpResponse();
    // 对资源请求进行相应
    void response(Buffer *buff, string &path, bool isKeepAlive, int code);
    //  获取文件资源共享内存的地址
    char *file() const;
    // 获取文件资源的长度
    size_t fileSize() const;
};

#endif // HTTP_RESPONSE_H
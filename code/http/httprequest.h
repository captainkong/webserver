#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "../buffer/buffer.h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <iostream>
#include <regex>

using std::string;

class HttpRequest
{
public:
    enum REQUEST_TYPE
    {
        GET,
        POST,
        OTHERS
    };

    HttpRequest();
    ~HttpRequest();

    // 解析用户请求
    bool prase(Buffer &read_buff);
    // 获取请求类型
    REQUEST_TYPE getRequestType() const;
    // 获取资源路径
    string getPath() const;
    // 获取版本号
    string getVersion() const;
    // 获取请求头参数
    string getHeader(const string &key) const;
    // 获取用户get传参
    string getGet(const string &key) const;
    // 获取用户post传参
    string getPost(const string &key) const;

private:
    enum PRASE_STATE
    {
        REQUEST,
        HEADER,
        BODY,
        FINISH
    };

    // 解析所处阶段
    PRASE_STATE state_;
    // 请求类型
    REQUEST_TYPE requestType_;
    // 请求元素
    string method_, path_, version_;
    // 记录GET传参
    std::unordered_map<string, string> get_;
    // 记录POST传参
    std::unordered_map<string, string> post_;
    // 记录headers
    std::unordered_map<string, string> headers_;

    // 解析请求行
    bool praseRequest(const string &line);
    // 解析请求头
    bool praseHeader(const string &line);
    // 解析请求体
    bool praseBody(const string &line);
    // 解析get/post的传参
    bool praseArg(const string &line);
};

#endif // HTTP_REQUEST_H
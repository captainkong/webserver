#include "httprequest.h"

using std::cout;
using std::endl;
#include <sys/time.h>

HttpRequest::HttpRequest() : state_(REQUEST), requestType_(OTHERS)
{
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::prase(Buffer &read_buff)
{
    // timeval startTime, endTime;
    // gettimeofday(&startTime, NULL);
    // 清空已有数据
    // method_ = path_ = version_ = "";
    // state_ = REQUEST;
    // headers_.clear();
    // post_.clear();
    // get_.clear();
    state_ = REQUEST;

    const char CRLF[] = "\r\n";
    if (read_buff.readableBytes() == 0)
        return false;
    while (state_ != FINISH)
    {
        // 首先从缓冲区中拿到该解析的数据
        const char *lineEnd = std::search(read_buff.beginRead(), read_buff.beginWriteConst(), CRLF, CRLF + 2);
        string line(read_buff.beginRead(), lineEnd);

        // 读指针后移
        read_buff.retrieve(line.size() + 2);
        switch (state_)
        {
        case REQUEST:
            if (!praseRequest(line))
                return false;
            break;
        case HEADER:
            if (line.size() == 0)
            {
                state_ = BODY;
                break;
            }
            praseHeader(line);
            if (read_buff.readableBytes() <= 2)
                state_ = FINISH;
            break;
        case BODY:
            if (!praseBody(line))
                return false;
            break;
        default:
            break;
        }
    }
    read_buff.retrieveAll();
    assert(state_ == FINISH);
    // cout << "解析成功!" << endl;

    // gettimeofday(&endTime, NULL);
    // int timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
    // cout << "第1阶段用时:" << timeUsed << "us" << endl;

    return true;
}

bool HttpRequest::praseRequest(const string &line)
{
    // 请求行的格式:  方法 资源路径 协议/版本
    // 使用正则表达式拿到请求参数
    // timeval startTime, endTime;
    // gettimeofday(&startTime, NULL);

    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch match;
    if (std::regex_match(line, match, pattern))
    {
        method_ = match[1];
        path_ = match[2];
        version_ = match[3];
        state_ = HEADER;
        cout << "method=" << method_ << ", path_=" << path_ << ", version=" << version_ << endl;

        // gettimeofday(&endTime, NULL);
        // int timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
        // cout << "praseRequest用时:" << timeUsed << "us" << endl;

        return true;
    }
    return false;
}

void HttpRequest::praseHeader(const string &line)
{
    // timeval startTime, endTime;
    // gettimeofday(&startTime, NULL);
    // Header的格式 key:_value
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch match;
    if (std::regex_match(line, match, pattern))
    {
        headers_[match[1]] = match[2];
        // cout << match[1] << ":" << match[2] << endl;
    }
    else
    {
        cout << "解析失败!" << endl;
    }

    // gettimeofday(&endTime, NULL);
    // int timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
    // cout << "praseHeader用时:" << timeUsed << "us" << endl;
}

bool HttpRequest::praseBody(const string &line)
{
    return false;
}

bool HttpRequest::praseArg(const string &line)
{
    return false;
}

HttpRequest::REQUEST_TYPE HttpRequest::getRequestType() const
{
    return GET;
}

string HttpRequest::getPath() const
{
    return path_;
}

string HttpRequest::getVersion() const
{
    return version_;
}

string HttpRequest::getHeader(const string &key) const
{
    auto it = headers_.find(key);
    if (it != headers_.end())
    {
        return it->second;
    }
    return "";
}

string HttpRequest::getGet(const string &key) const
{
    auto it = get_.find(key);
    if (it != get_.end())
    {
        return it->second;
    }
    return "";
}

string HttpRequest::getPost(const string &key) const
{
    auto it = post_.find(key);
    if (it != post_.end())
    {
        return it->second;
    }
    return "";
}
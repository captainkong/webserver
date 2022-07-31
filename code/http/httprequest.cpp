#include "httprequest.h"

using std::cout;
using std::endl;

HttpRequest::HttpRequest() : state_(REQUEST), requestType_(OTHERS)
{
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::prase(Buffer &read_buff)
{
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
            prasePath();
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
    assert(state_ == FINISH);
    cout << "解析成功!" << endl;
    return true;
}

bool HttpRequest::praseRequest(const string &line)
{
    // 请求行的格式:  方法 资源路径 协议/版本
    // 使用正则表达式拿到请求参数
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch match;
    if (std::regex_match(line, match, pattern))
    {
        method_ = match[1];
        path_ = match[2];
        version_ = match[3];
        state_ = HEADER;
        cout << "method=" << method_ << ", path_=" << path_ << ", version=" << version_ << endl;
        return true;
    }
    return false;
}

bool HttpRequest::prasePath()
{
    if (path_ == "/")
    {
        path_ = "/index.html";
    }
    cout << "HttpRequest::prasePath:" << path_ << endl;
    return true;
}

void HttpRequest::praseHeader(const string &line)
{
    // Header的格式 key:_value
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch match;
    if (std::regex_match(line, match, pattern))
    {
        headers_[match[1]] = match[2];
        cout << match[1] << ":" << match[2] << endl;
    }
    else
    {
        cout << "解析失败!" << endl;
    }
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
    cout << "HttpRequest::getPath() " << path_ << endl;
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
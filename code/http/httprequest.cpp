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
    // 清空已有数据
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
        read_buff.retrieve(line.size() + (state_ == BODY ? 0 : 2));
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
            praseBody(line);
            state_ = FINISH;
            break;
        default:
            break;
        }
    }
    read_buff.retrieveAll();
    assert(state_ == FINISH);
    // cout << "解析成功!" << endl;

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
        size_t inx = path_.find_first_of('?');
        if (inx != string::npos)
        {
            praseArg(path_.substr(inx + 1), true);
            path_ = path_.substr(0, inx);
            cout << "new path=" << path_ << endl;
        }
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
        state_ = BODY;
    }

    // gettimeofday(&endTime, NULL);
    // int timeUsed = 1000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
    // cout << "praseHeader用时:" << timeUsed << "us" << endl;
}

bool HttpRequest::praseBody(const string &line)
{
    cout << "HttpRequest::praseBody:" << line << endl;
    praseArg(line, false);
    // 使用post的几种可能 注册/登录/ajax请求
    // 根据请求路径判断
    if (path_ == "/register")
    {
        if (!regUser(post_["username"], post_["password"]))
            path_ = "/error.html";
        else
            path_ = "/welcome.html";
    }
    else if (path_ == "/login")
    {
        if (!login(post_["username"], post_["password"]))
            path_ = "/error.html";
        else
            path_ = "/welcome.html";
    }

    return false;
}

bool HttpRequest::praseArg(const string &line, bool isGet)
{
    // cout << "line:" << line << endl;
    // name=kang&age=25
    int n = line.size();
    string key, value;
    int sta = 0;
    for (int i = 0; i < n; ++i)
    {
        if (line[i] == '=')
        {
            sta = 1;
        }
        else if (line[i] == '&' || i == n - 1)
        {
            if (i == n - 1)
            {
                value += line[i];
            }
            // cout << "key=" << key << ",value=" << value << endl;
            if (isGet)
            {
                get_[key] = value;
            }
            else
            {
                post_[key] = value;
            }
            key = "";
            value = "";
            sta = 0;
            if (i == n - 1)
            {
                return true;
            }
        }
        else
        {
            if (sta == 0)
            {
                key += line[i];
            }
            else
            {
                value += line[i];
            }
        }
    }

    return false;
}

HttpRequest::REQUEST_TYPE HttpRequest::getRequestType() const
{
    if (method_ == "GET")
    {
        return GET;
    }
    else if (method_ == "POST")
    {
        return POST;
    }

    return OTHERS;
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

string HttpRequest::getArg(const string &key) const
{
    auto met = getRequestType();
    if (OTHERS == met)
        return "";
    if (GET == met)
    {
        return getGet(key);
    }
    else
    {
        return getPost(key);
    }
}

bool HttpRequest::getIsKeepAlive() const
{
    auto it = headers_.find("Connection");
    if (it != headers_.end())
    {
        return it->second == "keep-alive";
    }
    return false;
}

bool HttpRequest::regUser(const string &user_name, const string &password)
{
    if (user_name.size() == 0 || password.size() == 0)
        return false;

    MYSQL *sql;
    SqlRAII raii(&sql, SqlConnPool::getInstance());

    char query[128];
    snprintf(query, 128, "INSERT INTO `users` VALUES(NULL,\'%s\',\'%s\',now())", user_name.data(), password.data());
    cout << query << endl;
    if (mysql_real_query(sql, query, strlen(query)))
    {
        cout << "数据库写入失败!";
        return false;
    }
    return true;
}

bool HttpRequest::login(const string &user_name, const string &password)
{
    if (user_name.size() == 0 || password.size() == 0)
        return false;

    MYSQL *sql;
    SqlRAII raii(&sql, SqlConnPool::getInstance());

    char query[128];
    snprintf(query, 128, "SELECT `password` FROM `users` WHERE `name` = \'%s\' LIMIT 1", user_name.data());
    cout << query << endl;

    MYSQL_RES *res = nullptr;
    bool ans = false;

    do
    {
        if (mysql_real_query(sql, query, strlen(query)))
        {
            cout << "查询失败!";
            break;
        }
        res = mysql_store_result(sql);
        int n = mysql_num_fields(res);
        int m = res->row_count;
        if (m == 0)
        {
            cout << "无此用户" << endl;
            break;
        }
        assert(m == 1 && n == 1);
        MYSQL_ROW row = mysql_fetch_row(res);
        string psw = row[0];
        if (psw == password)
            ans = true;

    } while (false);

    // 释放资源
    mysql_free_result(res);
    return ans;
}

bool HttpRequest::isExistsUser(const string &user_name)
{
    // 暂时不实现,放在response中实现
    return false;
}
#include "httpresponse.h"
#include <iostream>

using std::cout;
using std::endl;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {".js", "text/javascript"},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse(string path) : rootDir_(path), mmFile_(nullptr)
{
}

HttpResponse::~HttpResponse()
{
    unmapFile();
}

void HttpResponse::response(HttpRequest *request, Buffer *buff, string &path, bool isKeepAlive, int code)
{
    cout << "HttpResponse::response  path:" << path << ",code:" << code << endl;
    pBuff_ = buff;
    path_ = path;
    isKeepAlive_ = isKeepAlive;
    statusCode_ = code;
    httpRequest_ = request;

    if (path == "/api")
    {
        responseAPI();
        return;
    }
    prasePath();

    string absDir = rootDir_ + path_;
    cout << "absDir=" << absDir << endl;
    if (statusCode_ == 200)
    {
        // 状态码为200只能说明请求解析成功,并不能说明资源可用
        int ret = stat(absDir.data(), &fileStat_);
        int fileFd = open((absDir).data(), O_RDONLY);
        if (ret != 0 || fileFd < 0)
        {
            statusCode_ = 404;
        }
        else if ((!(fileStat_.st_mode & S_IROTH)) || S_ISDIR(fileStat_.st_mode))
        {
            statusCode_ = 403;
        }
        else
        {
            int *mmRet = (int *)mmap(0, fileStat_.st_size, PROT_READ, MAP_PRIVATE, fileFd, 0);
            if (*mmRet == -1)
            {
                statusCode_ = 404;
            }
            else
            {
                mmFile_ = (char *)mmRet;
            }
        }
        close(fileFd);
    }

    if (statusCode_ != 200)
    {
        auto it = CODE_STATUS.find(statusCode_);
        if (it == CODE_STATUS.end())
        {
            statusCode_ = 400;
        }
        string errorPage = CODE_PATH.find(statusCode_)->second;
        string absPath = rootDir_ + errorPage;
        cout << "absPath:" << absPath << endl;
        int ret = stat(absPath.data(), &fileStat_);
        assert(ret == 0);
    }

    makeStatusLine();
    makeHeader();
    makeBody();
}

void HttpResponse::responseAPI()
{
    cout << "HttpResponse::responseAPI()";
    string reqType = httpRequest_->getArg("type");
    // cout << "reqType=" << reqType << endl;
    if (reqType == "ajaxVerifyRegName")
    {
        string name = httpRequest_->getArg("name");
        cout << "name=" << name << endl;
        // cout << "可写长度:" << pBuff_->readableBytes() << endl;
        makeStatusLine();
        makeHeader();
        string res = "verifyOk";
        pBuff_->append("Content-Length: " + std::to_string(res.size()) + "\r\n\r\n" + res);
        // cout << "可写长度:" << pBuff_->readableBytes() << endl;
    }
}

void HttpResponse::prasePath()
{
    if (path_ == "/")
    {
        path_ = "/index.html";
    }
    // cout << "HttpRequest::prasePath:" << path_ << endl;
}

void HttpResponse::makeStatusLine()
{
    // HTTP/1.1   200   OK
    string status;
    auto it = CODE_STATUS.find(statusCode_);
    assert(it != CODE_STATUS.end());
    string sl = "HTTP/1.1 " + std::to_string(statusCode_) + " " + status + "\r\n";
    pBuff_->append(sl);
}

void HttpResponse::makeHeader()
{
    pBuff_->append("Connection: ");
    if (isKeepAlive_)
    {
        pBuff_->append("keep-Alive\r\n");
        pBuff_->append("keep-Alive: max=6, timeout=120\r\n");
    }
    else
    {
        pBuff_->append("close\r\n");
    }
    pBuff_->append("Server: CaptainKong's Server/0.1(Ubuntu)\r\n");
    // 添加文件类型
    pBuff_->append("Content-Type: " + getFileType() + "\r\n");
}

void HttpResponse::makeBody()
{
    // 添加长度字段
    if (statusCode_ == 200)
    {
        assert(mmFile_);
    }
    else
    {
        auto it = CODE_PATH.find(statusCode_);
        assert(it != CODE_PATH.end());
        string absDir = rootDir_ + it->second;
        int fileFd = open((absDir).data(), O_RDONLY);
        int *mmRet = (int *)mmap(0, fileStat_.st_size, PROT_READ, MAP_PRIVATE, fileFd, 0);
        assert(*mmRet != -1);
        mmFile_ = (char *)mmRet;
        close(fileFd);
    }
    // 添加内容长度信息 + 空行
    pBuff_->append("Content-Length: " + std::to_string(fileStat_.st_size) + "\r\n\r\n");
}

string HttpResponse::getFileType() const
{
    if (path_ == "/api")
        return "text/plain";
    static string defaultSuffix = "text/html";
    size_t index = path_.find_last_of('.');
    if (index == std::string::npos)
    {
        return defaultSuffix;
    }
    string suffix = path_.substr(index);
    auto it = SUFFIX_TYPE.find(suffix);
    if (it != SUFFIX_TYPE.end())
        return it->second;
    return defaultSuffix;
}

char *HttpResponse::file() const
{
    return mmFile_;
}

size_t HttpResponse::fileSize() const
{
    return fileStat_.st_size;
}

void HttpResponse::unmapFile()
{
    if (mmFile_)
    {
        munmap(mmFile_, fileStat_.st_size);
        mmFile_ = nullptr;
    }
}
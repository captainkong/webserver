#include "webserver.h"

using std::cout;
using std::endl;

WebServer::WebServer(int thread_size, int port_number, int max_user_count) : epoll_(max_user_count), pool_(thread_size), isClose_(false)
{
    // 指定网站的根目录
    char *path = getcwd(nullptr, 256);
    assert(path);
    strncat(path, "/www", 5);
    HttpConnect::wwwRoot = path;

    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;

    int ret;
    struct sockaddr_in addr;

    // 创建套接字
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenFd_ != -1);

    // 设置端口复用
    int opt = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEPORT, (void *)&opt, sizeof(opt));

    // 绑定
    addr.sin_port = htons(port_number);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(0);
    socklen_t len = sizeof(addr);
    ret = bind(listenFd_, (sockaddr *)&addr, len);
    assert(ret != -1);

    // 监听
    ret = listen(listenFd_, max_user_count);
    epoll_.addFd(listenFd_, EPOLLIN | EPOLLET);
}

WebServer::~WebServer()
{
}

void WebServer::acceptNewClient()
{
    struct sockaddr_in cliAddr;
    socklen_t len = sizeof(cliAddr);
    int cfd = accept(listenFd_, (struct sockaddr *)&cliAddr, &len);
    if (cfd == -1)
    {
        perror("连接建立失败!");
    }

    // 将cfd设置为非阻塞模式
    int flags = fcntl(cfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flags);

    char ip[16];
    inet_ntop(AF_INET, &cliAddr.sin_addr, ip, sizeof(ip));
    HttpConnect *newConn = new HttpConnect(cfd, cliAddr);
    users_[cfd] = newConn;
    printf("New Connect: %s:%d\n", newConn->getIP(), newConn->getPort());
    // 将新的连接添加到红黑树中
    int ret = epoll_.addFd(cfd, EPOLLIN | EPOLLET);
    assert(ret);
}

void WebServer::dealClientRead(int cfd)
{
    HttpConnect *client = users_[cfd];
    assert(client);
    int err = 0;
    size_t len = client->readFromClient(&err);
    cout << "读了" << len << "字节数据" << endl;
    if (len <= 0 && err != EAGAIN)
    {
        closeConnect(client);
        return;
    }

    // 开始处理请求
    onProcess(client);
}

void WebServer::onProcess(HttpConnect *client)
{
    if (client->praseRequest())
    {
        epoll_.modFd(client->getFd(), connEvent_ | EPOLLOUT);
    }
    else
    {
        epoll_.modFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::dealClientWrite(int cfd)
{
    HttpConnect *client = users_[cfd];
    assert(client);
    int err = 0;
    size_t len = client->sendToClient(&err);
    cout << "webServer:len" << len << ",err:" << err << ",WriteableBytes:" << client->getWriteableBytes() << endl;
    if (client->getWriteableBytes() == 0)
    {
        if (client->isKeepAlive())
        {
            onProcess(client);
            return;
        }
        else
        {
            cout << "not keepAlive()" << endl;
        }
    }
    else if (len < 0)
    {
        if (err == EAGAIN)
        {
            epoll_.modFd(client->getFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    closeConnect(client);
}

void WebServer::start()
{
    // 事件循环
    while (!isClose_)
    {
        int ready = epoll_.wait();
        for (int i = 0; i < ready; ++i)
        {
            u_int32_t events = epoll_.getEvent(i);
            int fd = epoll_.getFd(i);

            if (fd == listenFd_)
            {
                // 新的连接到来  任务交给线程池处理
                pool_.addTask(std::bind(&WebServer::acceptNewClient, this));
            }
            else if (events & EPOLLIN)
            {
                cout << "启动新的io线程,准备读客户端" << endl;
                // 处理客户端消息 任务交给线程池处理
                pool_.addTask(std::bind(&WebServer::dealClientRead, this, epoll_.getFd(i)));
            }
            else if (events & EPOLLOUT)
            {
                cout << "启动新的io线程,准备写客户端" << endl;
                pool_.addTask(std::bind(&WebServer::dealClientWrite, this, epoll_.getFd(i)));
            }
        }
    }
}

void WebServer::closeConnect(HttpConnect *con)
{
    int cfd = con->getFd();
    cout << "Closed: " << con->getIP() << ":" << con->getPort() << endl;
    users_.erase(cfd);
    close(cfd);
    epoll_.delFd(cfd);
    cout << "下树完毕" << endl;
    delete con;
    // 可能不准(不一致)
    cout << " 剩余客户端:" << users_.size() << endl;
}
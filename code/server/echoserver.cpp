#include "echoserver.h"

EchoServer::EchoServer(int thread_size, int port_number, int max_user_count)
    : epoll_(max_user_count), pool_(thread_size), isClose_(false)
{
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

EchoServer::~EchoServer()
{
    isClose_ = true;
    close(listenFd_);
}

void EchoServer::start()
{
    // 事件循环
    while (!isClose_)
    {
        int ready = epoll_.wait();
        for (int i = 0; i < ready; ++i)
        {
            if (epoll_.getFd(i) == listenFd_ && epoll_.getEvent(i) & EPOLLIN)
            {
                // 新的连接到来  任务交给线程池处理
                pool_.addTask(std::bind(&EchoServer::acceptNewClient, this));
            }
            else if (epoll_.getFd(i) != listenFd_ && epoll_.getEvent(i) & EPOLLIN)
            {
                // 处理客户端消息 任务交给线程池处理
                pool_.addTask(std::bind(&EchoServer::readFromClient, this, i));
            }
        }
    }
}

void EchoServer::acceptNewClient()
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
    client_t *newConn = new client_t(ntohs(cliAddr.sin_port), ip);
    users_[cfd] = newConn;
    printf("New Connect: %s:%d\n", newConn->ip, newConn->port);
    // 将新的连接添加到红黑树中
    int ret = epoll_.addFd(cfd, EPOLLIN | EPOLLET);
    assert(ret);
}

void EchoServer::readFromClient(int index)
{
    int clientFd = epoll_.getFd(index);
    client_t *client = users_[clientFd];
    int err = 0;
    while (true)
    {
        int n = client->readBuffer.readFd(clientFd, &err);
        cout << "n=" << n << ", err=" << err << ", readable=" << client->readBuffer.readableBytes() << endl;
        if (n < 1)
        {
            if (err == EAGAIN)
            {
                break;
            }
            // 出现错误或者对方主动关闭 下树
            client_t *tem = users_[clientFd];
            cout << "Closed: " << tem->ip << ":" << tem->port;
            delete tem;
            users_.erase(clientFd);
            close(clientFd);
            epoll_.delFd(clientFd);
            cout << " 剩余客户端:" << users_.size() << endl;
            return;
        }
    }

    // 目前读和处理粘在一起, 对于echo 服务可以接受
    string str = client->readBuffer.retrieveAllToString();
    for (int j = 0; j < static_cast<int>(str.size()); ++j)
    {
        if (str[j] >= 'a' && str[j] <= 'z')
        {
            str[j] -= 'a' - 'A';
        }
    }
    send(clientFd, str.data(), str.size(), 0);
    if (str[str.size() - 1] == '\n')
        str[str.size() - 1] = '\0';
    cout << "服务端转发:" << client->ip << ":" << client->port << " len=" << str.size() << endl;
}
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>

#include <transport/ClientTransport.hh>

#include <runtime/handlemodel/EventHandlerManager.hh>

#include <serialization/Serializer.hh>

#include <common/Defines.hh>
#include <common/Logger.hh>

static int ConnectTo(std::string ip, int port)
{
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        log_dev("connection failed\n");
        close(sockfd);
    }

    return sockfd;
}

ClientTransport::ClientTransport(): _poller(false), Connfd(-1), PendingRequests() {}

ClientTransport::~ClientTransport()
{
    close(Connfd);
}

void ClientTransport::HandleCloseEvent(int Fd)
{
    log_err("Server close");
    exit(1);
}

void ClientTransport::HandleWriteEvent(int Fd)
{
    assert(Fd == Connfd);

    std::unique_lock<std::mutex> lock(mu); // serialize Push() and HandleWriteEvent()

    if (!PendingRequests.empty())
    {
        Send(PendingRequests.front());
        PendingRequests.pop();
    }
    // 如果当前没有等待发送的Rpc，则关闭EPOLLOUT
    if (PendingRequests.empty()) 
    {
        _poller.ModEvent(Connfd, EPOLLIN | EPOLLERR | EPOLLRDHUP);
    }
}

int ClientTransport::Connect(std::string ip, int port)
{
    Connfd = ConnectTo(ip, port);
    if (Connfd < 0)
    {
        log_err("Connect failed");
        exit(1);
    }
    // 监听所有事件
    _poller.AddEvent(Connfd, EPOLLIN | EPOLLERR | EPOLLRDHUP);
    return Connfd;
}

void ClientTransport::Push(const RpcMessage& Message) 
{
    assert(Connfd != -1);

    std::unique_lock<std::mutex> lock(mu); // serialize Push() and HandleWriteEvent()

    // 新请求入队
    PendingRequests.push(Message);
    // 打开EPOLLOUT
    _poller.ModEvent(Connfd, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP);
}

void ClientTransport::Send(const RpcMessage& Message)
{
    char buf[MAX_BUFFER];
    int size = Serializer::Serialize(&Message, buf);
    log_dev("body_length = %d\n", Message.header.body_length);
    log_dev("size = %d\n", size);

    // send msg, it would not block
    int ret = send(Connfd, buf, size, 0);
    if (ret < 0)
    {
        log_dev("ClientTransport::Send: Send message failed\n");
        return;
    }
}

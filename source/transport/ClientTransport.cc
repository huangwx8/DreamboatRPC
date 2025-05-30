#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>

#include <transport/ClientTransport.hh>

#include <runtime/handlemodel/EventHandlerManager.hh>

#include <serialization/Serializer.hh>

#include <common/Defines.hh>
#include <common/Logger.hh>

static int ConnectTo(const std::string& ip, int port) {
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &server_address.sin_addr) <= 0) {
        log_err("Invalid IP address: %s\n", ip.c_str());
        return -1;
    }
    server_address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_err("Socket creation failed: %s\n", strerror(errno));
        return -1;
    }

    // Set the socket buffer size (both send and receive)
    int bufferSize = 1024 * 1024;  // 1 MB buffer size, change as needed
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) < 0) {
        log_err("Failed to set receive buffer size: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) < 0) {
        log_err("Failed to set send buffer size: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        log_err("Connect failed: %s\n", strerror(errno));
        close(sockfd);
        return -1;
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
        _poller.ModEvent(Connfd, Poller::EPOLL_FLAGS_IN);
    }
}

int ClientTransport::Connect(std::string ip, int port)
{
    Connfd = ConnectTo(ip, port);
    if (Connfd < 0)
    {
        exit(1);
    }
    // 监听所有事件
    _poller.AddEvent(Connfd, Poller::EPOLL_FLAGS_IN);
    return Connfd;
}

void ClientTransport::Push(const RpcMessage& Message) 
{
    assert(Connfd != -1);

    std::unique_lock<std::mutex> lock(mu); // serialize Push() and HandleWriteEvent()

    // 新请求入队
    PendingRequests.push(Message);
    // 打开EPOLLOUT
    _poller.ModEvent(Connfd, Poller::EPOLL_FLAGS_INOUT);
}

void ClientTransport::Send(const RpcMessage& Message)
{
    char buf[MAX_BUFFER];
    int size = Serializer::Serialize(&Message, buf);

    // send msg, it would not block
    int ret = send(Connfd, buf, size, 0);
    if (ret < 0)
    {
        log_dev("ClientTransport::Send: Send message failed\n");
        return;
    }
}

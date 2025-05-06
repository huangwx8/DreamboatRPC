// std c/c++
#include <assert.h>
#include <stdio.h>
#include <string.h>

// linux
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// inner
#include <transport/ServerTransport.hh>
#include <common/Logger.hh>

// c-style creator
static int create_listenfd(std::string ip, int port)
{
    int ret = 0;
    // net data
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    address.sin_port = htons(port);
    // create socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    // bind
    ret = bind(listenfd, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    // listen
    ret = listen(listenfd, 5);
    assert(ret != -1);

    return listenfd;
}

ServerTransport::ServerTransport(
    EventHandler* SocketReader,
    EventHandler* SocketWriter
):
    _socket_reader(SocketReader),
    _socket_writer(SocketWriter),
    _event_handler_manager(),
    _reactor(&_event_handler_manager)
{
   
}

ServerTransport::~ServerTransport()
{
    if (_listenfd >= 0)
    {
        HandleCloseEvent(_listenfd);
    }
}

void ServerTransport::Listen(std::string ip, int port)
{
    _listenfd = create_listenfd(ip, port);
    if (_listenfd >= 0)
    {
        _event_handler_manager.AttachEventHandler(_listenfd, EventHandler::READ_EVENT, this);
        _reactor.GetPoller().AddEvent(_listenfd, Poller::EPOLL_FLAGS_IN_ONESHOT);
        log_dev("ServerTransport::Listen: Start listening at ip %s, port %d\n", ip.c_str(), port);
    }
    else 
    {
        log_err("Listen failed");
        exit(1);
    }
}

void ServerTransport::HandleReadEvent(int Fd)
{
    if (Fd != _listenfd) {
        log_err("ServerTransport::HandleReadEvent listen at a wrong fd!\n");
        exit(1);
    }
    int Connfd = Accept();
    _event_handler_manager.AttachEventHandler(Connfd, EventHandler::READ_EVENT, _socket_reader);
    _event_handler_manager.AttachEventHandler(Connfd, EventHandler::WRITE_EVENT, _socket_writer);
    _event_handler_manager.AttachEventHandler(Connfd, EventHandler::CLOSE_EVENT, this);
    _reactor.GetPoller().AddEvent(Connfd, Poller::EPOLL_FLAGS_IN_ONESHOT);
    _reactor.GetPoller().ModEvent(_listenfd, Poller::EPOLL_FLAGS_IN_ONESHOT);
}

void ServerTransport::HandleCloseEvent(int Fd)
{
    log_dev("ServerTransport::HandleCloseEvent Close fd [%d]\n", Fd);
    _event_handler_manager.DetachEventHandler(Fd, static_cast<EventHandler::EventType>(
            EventHandler::READ_EVENT | EventHandler::WRITE_EVENT | EventHandler::CLOSE_EVENT
        )
    );
    _reactor.GetPoller().DelEvent(Fd, 0);
    close(Fd);
}

int ServerTransport::Accept()
{
    // Accept a new TCP connection request
    struct sockaddr_in ClientAddress;
    socklen_t ClientAddrLength = sizeof(ClientAddress);
    int Connfd = accept(_listenfd, (struct sockaddr*)&ClientAddress, &ClientAddrLength);

    if (Connfd < 0)
    {
        log_dev("ServerTransport::Accept: Accept failure, errno is %d\n", errno);
        return -1;
    }

    // Enlarge the socket buffer size (both receive and send buffers)
    int bufferSize = 1024 * 1024;  // 1 MB buffer size, change as needed

    if (setsockopt(Connfd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) < 0)
    {
        log_dev("ServerTransport::Accept: Failed to set receive buffer size: %s\n", strerror(errno));
        close(Connfd);
        return -1;
    }

    if (setsockopt(Connfd, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) < 0)
    {
        log_dev("ServerTransport::Accept: Failed to set send buffer size: %s\n", strerror(errno));
        close(Connfd);
        return -1;
    }

    log_dev("ServerTransport::Accept: Created a connection at fd [%d], buffer size set to %d bytes\n", Connfd, bufferSize);
    return Connfd;
}

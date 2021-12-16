// std c/c++
#include <assert.h>
#include <stdio.h>
#include <string.h>

// linux
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// inner
#include <transport/ServerConnectionManager.hh>

// c-style creator
static int CreateListenFd(const char* ip, int port)
{
    int ret = 0;
    // net data
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
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

ServerConnectionManager::ServerConnectionManager(
    std::function<void(int, bool)> RegisterEvent,
    std::function<void(int)> UnregisterEvent
):
    OnPostOpenFd(RegisterEvent),
    OnPreCloseFd(UnregisterEvent)
{
   
}

ServerConnectionManager::~ServerConnectionManager()
{
    if (ListenFd >= 0)
    {
        HandleCloseEvent(ListenFd);
    }
}

void ServerConnectionManager::Listen()
{
    ListenFd = CreateListenFd("localhost", 8888);
    if (ListenFd >= 0)
    {
        OnPostOpenFd(ListenFd, true);
        printf("ServerConnectionManager::Listen: Start listening at fd [%d]\n", ListenFd);
    }
    else 
    {
        throw "Listen failed";
    }
}

void ServerConnectionManager::HandleReadEvent(int Fd)
{
    int Connfd = Accept();
    OnPostOpenFd(Connfd, false);
}

void ServerConnectionManager::HandleCloseEvent(int Fd)
{
    printf("ServerConnectionManager::HandleCloseEvent Close fd [%d]\n", Fd);
    OnPreCloseFd(Fd);
    close(Fd);
}

int ServerConnectionManager::Accept()
{
    // accept a new tcp connection request
    struct sockaddr_in ClientAddress;
    socklen_t ClientAddrLength = sizeof(ClientAddress);
    int Connfd = accept(ListenFd, (struct sockaddr*)&ClientAddress, &ClientAddrLength);
    if (Connfd < 0)
    {
        printf("ServerConnectionManager::Accept: Accept failure, errno is %d\n", errno);
        return -1;
    }
    printf("ServerConnectionManager::Accept: Create a connection at fd [%d]\n", Connfd);
    return Connfd;
}

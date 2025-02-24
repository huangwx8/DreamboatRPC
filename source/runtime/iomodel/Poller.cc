// std c/c++
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


// linux
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

// inner
#include <runtime/iomodel/Poller.hh>
#include <runtime/handlemodel/EventHandler.hh>

#include <common/Logger.hh>

#define MAX_EVENTS 10240

static int setnonblocking(int fd) 
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

uint32_t Poller::EPOLL_FLAGS_IN = EPOLLIN | EPOLLERR | EPOLLRDHUP;
uint32_t Poller::EPOLL_FLAGS_IN_ONESHOT = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLONESHOT;
uint32_t Poller::EPOLL_FLAGS_INOUT = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
uint32_t Poller::EPOLL_FLAGS_INOUT_ONESHOT = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP | EPOLLONESHOT;

Poller::Poller(bool et)
    : Epollfd(epoll_create(1024)),
      MaxEvents(MAX_EVENTS),
      ExtraFlags(0),
      Events(new epoll_event[MaxEvents + 1])
{
    assert(Epollfd > 0);
    if (et)
    {
        ExtraFlags = EPOLLET;
    }
}

Poller::~Poller() 
{
    if (Events) 
    {
        delete[] Events;
        Events = nullptr;
    }
    if (Epollfd > 0) 
    {
        close(Epollfd);
    }
}

std::vector<std::function<void()>> Poller::Dispatch(int Timeout, EventHandler& Handler)
{
    std::vector<std::function<void()>> tasks;

    int NumEvents = epoll_wait(Epollfd, Events, MaxEvents + 1, Timeout);

    for (int i = 0; i < NumEvents; i++)
    {
        int Sockfd = Events[i].data.fd;
        uint32_t EventType = Events[i].events;
        int Type = 0;
        if (EventType & EPOLLERR)
        {
            log_dev("Poller::Dispatch: Got an error from fd [%d]\n", Sockfd);
            char errors[100];
            memset(errors, '\0', 100);
            socklen_t length = sizeof(errors);
            if (getsockopt(Sockfd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
            {
                log_dev("Poller::Dispatch: Get socket option failed\n");
            }
            else
            {
                log_dev("Poller::Dispatch: Errormsg is %s\n", errors);
            }
        }
        else if (EventType & EPOLLRDHUP)
        {
            Type = EventHandler::CLOSE_EVENT | Type;
        }
        else if (EventType & EPOLLIN)
        {
            Type |= EventHandler::READ_EVENT;
        }
        else if (EventType & EPOLLOUT)
        {
            Type |= EventHandler::WRITE_EVENT;
        }
        else
        {
            log_err("Unexcepted event");
            exit(1);
        }
        tasks.push_back(
            [&Handler, Sockfd, Type]()
            {
                Handler.HandleEvent(Sockfd, static_cast<EventHandler::EventType>(Type));
            }
        );
    }
    return tasks;
}

void Poller::AddEvent(int Fd, uint32_t InEvents)
{
    setnonblocking(Fd);
    CtrlEvent(Fd, InEvents, EPOLL_CTL_ADD);
}

void Poller::DelEvent(int Fd, uint32_t InEvents)
{
    CtrlEvent(Fd, InEvents, EPOLL_CTL_DEL);
}

void Poller::ModEvent(int Fd, uint32_t InEvents)
{
    CtrlEvent(Fd, InEvents, EPOLL_CTL_MOD);
}

void Poller::CtrlEvent(int Fd, uint32_t InEvents, int Operation)
{
    epoll_data_t data{.fd = Fd};
    epoll_event event{.events = InEvents | ExtraFlags, .data = data};
    epoll_ctl(Epollfd, Operation, Fd, &event);
}

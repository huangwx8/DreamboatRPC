#pragma once

// std c++
#include <vector>
#include <functional>

// linux
#include <sys/types.h>

#include "runtime/handlemodel/EventHandler.hh"

/**
 * epoll multiplexing event trigger
 */
class Poller
{
public:
    static uint32_t EPOLL_FLAGS_IN;
    static uint32_t EPOLL_FLAGS_IN_ONESHOT;
    static uint32_t EPOLL_FLAGS_INOUT;
    static uint32_t EPOLL_FLAGS_INOUT_ONESHOT;

    Poller(bool et = false);
    ~Poller();
    std::vector<std::function<void()>> Dispatch(int Timeout, EventHandler &Handler);
    void AddEvent(int Fd, uint32_t InEvents);
    void DelEvent(int Fd, uint32_t InEvents);
    void ModEvent(int Fd, uint32_t InEvents);

private:
    void CtrlEvent(int Fd, uint32_t InEvents, int Operation);
    // epoll fd
    int Epollfd;
    // max num of connections
    int MaxEvents;
    // extra epoll flags, EPOLLET
    int ExtraFlags;
    // interest set
    struct epoll_event* Events;
};

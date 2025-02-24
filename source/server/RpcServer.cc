// std
#include <functional>
#include <assert.h>
#include <cstring>
#include <string>
#include <thread>

// inner
#include <server/RpcServer.hh>

#include <common/Logger.hh>

void RpcServer::FileDescriptorEventDone(int fd) 
{
    if (_pipe.Empty(fd))
    {
        _transport.GetReactor().GetPoller().ModEvent(fd, Poller::EPOLL_FLAGS_IN_ONESHOT);
    }
    else 
    {
        _transport.GetReactor().GetPoller().ModEvent(fd, Poller::EPOLL_FLAGS_INOUT_ONESHOT);
    }
}

RpcServer::RpcServer(Options options):
    _options(options),
    _pipe(),
    _transport(&_receiver, &_sender),
    _receiver(&_pipe, this),
    _sender(&_pipe, this)
{
    start_log(options.log_path.c_str());
}

RpcServer::~RpcServer()
{
    stop_log();
}

void RpcServer::RegisterService(RpcServiceBase* Service)
{
    _receiver.AddProxy(Service);
}

int RpcServer::Main(int argc, char* argv[])
{
    // Listen to clients' connect()
    _transport.Listen(_options.ip_addr, _options.port);
    // poller select several events, deliver them to main handler, and then repeat
    _transport.GetReactor().Run();

    return 0;
}

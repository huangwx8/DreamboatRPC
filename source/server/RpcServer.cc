// std
#include <functional>
#include <assert.h>
#include <cstring>
#include <string>
#include <thread>
#include <future>
#include <unistd.h>

// inner
#include <server/RpcServer.hh>

#include <common/Logger.hh>

#include <common/RpcServiceBase.hh>

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
    _sender(&_pipe, this),
    _reporter(nullptr)
{
    start_log(options.log_path.c_str());

    _reporter = ServiceReporter::GetServiceReporter(
        {.svr_ip=_options.namesvr_ip_addr, .svr_port = _options.namesvr_port});
}

RpcServer::~RpcServer()
{
    stop_log();
}

void RpcServer::RegisterService(RpcServiceBase* Service)
{
    _receiver.AddProxy(Service);

    _reporter->RegisterMyself(
        _options.ip_addr, 
        _options.port, 
        Service->GetServiceName());
}

int RpcServer::Main(int argc, char* argv[])
{
    // Start a heartbeat thread
    std::thread([&](){
        while (true) {
            bool success = _reporter->SendHeartbeat(_options.ip_addr, _options.port);
            if (!success)
            {
                log_err("Heartbeat failed!\n");
                exit(1);
            }
            sleep(5);
        }
    }).detach();

    // Listen to clients' connect()
    _transport.Listen(_options.ip_addr, _options.port);
    // poller select several events, deliver them to main handler, and then repeat
    _transport.GetReactor().Run();

    return 0;
}

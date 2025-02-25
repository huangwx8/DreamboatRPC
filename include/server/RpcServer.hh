#pragma once

#include <queue>
#include <array>

#include <common/Defines.hh>
#include <common/RpcTypes.hh>

#include <transport/ServerTransport.hh>

#include <server/RpcEventHandler.hh>

class RpcServiceBase;
class ServerTransport;
class RpcRequestHandler;
class RpcResultSender;
class Reactor;

class RpcServer: public FileDescriptorEventDelegate
{
public:
    struct Options
    {
        std::string namesvr_ip_addr;
        int namesvr_port;
        std::string ip_addr;
        int port;
        std::string log_path;
    };
public:
    virtual void FileDescriptorEventDone(int fd) override;
public:
    RpcServer(Options);
    ~RpcServer();
    RpcServer(RpcServer&) = delete;
    RpcServer(RpcServer&&) = delete;
    /**
     * Register a new custom service object
     */
    void RegisterService(RpcServiceBase* Service);
    /**
     * Main entry
     */
    int Main(int argc, char* argv[]);

private:
    // settings
    Options _options;

    // buffer
    ReturnValuePipe _pipe;

    // workers
    ServerTransport _transport;
    RpcRequestHandler _receiver;
    RpcResultSender _sender;
};

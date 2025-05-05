#pragma once

#include <atomic>
#include <mutex>
#include <functional>
#include <string>
#include <condition_variable>

#include <common/RpcTypes.hh>

#include <runtime/handlemodel/EventHandlerManager.hh>
#include <runtime/iomodel/Poller.hh>

#include <transport/ClientTransport.hh>

#include <client/CallbacksHandler.hh>

class Poller;
class RpcServiceBase;
class EventHandlerRouter;
class ClientTransport;
class CallbacksHandler;

class RpcClient
{
public:
    struct Options
    {
        std::string callee;
        std::string svr_ip;
        int svr_port;
    };

public:
    RpcClient(Options);
    ~RpcClient();
    RpcClient(RpcClient&) = delete;
    RpcClient(RpcClient&&) = delete;
    /** 
     * Construct and initialize
     */
    static std::shared_ptr<RpcClient> GetRpcClient(Options options);
    /**
     * Proxy creator
     */
    template<typename T, typename... ArgumentTypes>
    std::shared_ptr<T> GetProxy(ArgumentTypes... Arguments)
    {
        std::shared_ptr<T> proxy_ptr = std::make_shared<T>(Arguments...);
        proxy_ptr->client = this;
        return proxy_ptr;
    }

    /** 
     * client send rpc request
     * Not reentrant now, consider to use task queue in the future
     */
    void SendRequest(const RpcMessage& Message);

    CallbacksHandler& GetCallbacksHandler() { return _callback_handler; }

private:
    /** 
     * initialize client
     */
    void Initialize();
    /** 
     * client's main procedure
     */
    int Main(int argc, char* argv[]);
    
    Options _options;

    // worker
    ClientTransport _transport;
    EventHandlerRouter _router;
    CallbacksHandler _callback_handler;

    // concurrency
    mutable std::mutex m{};
    std::atomic<bool> _initialized;
    std::atomic<bool> m_stop;
};

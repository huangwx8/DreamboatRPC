#pragma once

// std
#include <string>
#include <unordered_map>
#include <functional>

// inner
#include <runtime/handlemodel/EventHandler.hh>

class RpcServiceProxy;

class RpcRequestHandler: public EventHandler
{
public:
    RpcRequestHandler(std::function<void(int, int, int)>);
    virtual ~RpcRequestHandler() = default;
    /**
     * If one EPOLLIN triggered at a connfd, try find the matching rpc implematation to process
     */
    virtual void HandleReadEvent(int Fd) override;
    /**
     * Add a rpc service implementation object into hash map, it can be found in next HandleReadEvent
     */
    void AddProxy(RpcServiceProxy* Service);
private:
    // Name-Service Mapping
    std::unordered_map<std::string, RpcServiceProxy*> RpcServiceDict;

    std::function<void(int, int, int)> OnFinishTask;
};

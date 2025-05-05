#pragma once

#include <functional>
#include <common/RpcTypes.hh>
#include <common/RpcUtils.hh>

#ifdef BUILD_CLIENT
#include <client/RpcClient.hh>
#include <client/CallbacksHandler.hh>
#endif

#ifdef BUILD_SERVER
#define HandleRPC(ReqType, RspType, F)\
    ReqType req;\
    ParseProtoStruct(&(Context.body.parameters[0]), req);\
    RspType rsp = F(req);\
    return ToRpcResult(rsp);
#endif

class RpcServiceBase
{
    /** Common section */
public:
    RpcServiceBase() {};
    virtual ~RpcServiceBase() = default;
    const char* GetServiceName() { return ServiceName; };
protected:
    const char* ServiceName = "None";

#ifdef BUILD_SERVER
    /** Server section */
public:
    virtual RpcResult Handle(const RpcMessage& Context) = 0;
#endif

#ifdef BUILD_CLIENT
friend RpcClient;
    /** Client section */
public:
    template<typename ParamType, typename RetType>
    void CallRPCAsync(ParamType P, std::function<void(RetType)> Callback)
    {
        RpcMessage __RpcMessage;
        __RpcMessage.header.seqno = Guid::GetGuid();
        if (__RpcMessage.header.seqno == -1) {
            return;
        }
        strcpy(__RpcMessage.header.servicename, GetServiceName());
        __RpcMessage.header.body_length = PackProtoStruct(&(__RpcMessage.body.parameters[0]), P);
        AsyncInvoke(__RpcMessage, Callback);
    }

    template<typename RetType, typename ParamType>
    RetType CallRPC(ParamType P)
    {
        std::promise<RetType> p;
        std::future<RetType> f = p.get_future();
        std::function<void(RetType)> cb = [&p](RetType ret) { p.set_value(ret); };
        CallRPCAsync(P, cb);
        return f.get();
    }

private:
    void Invoke(const RpcMessage& Context)
    {
        if (client) 
        {
            client->SendRequest(Context);
        }
    }

    template<typename T>
    void AsyncInvoke(RpcMessage& Context, std::function<void(T)> func)
    {
        if (client) 
        {
            // 注册回调
            client->GetCallbacksHandler().Register(Context.header.seqno, func);
            // 发送报文
            Context.header.need_return = true;
            client->SendRequest(Context);
        }
    }
private:
    RpcClient* client = nullptr;
#endif
};

#include <functional>

#include <apps/kv/KVClient.hh>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>

GetValueRsp KVGetterProxy::GetValue(GetValueReq req)
{
    std::function<void(GetValueRsp)> cb = [=](GetValueRsp s) {
        printf("GetValue(%d) from server: %s\n", req.id(), s.name().c_str());
    };
    CallRPCAsync(req, cb);
    return {};
}

SetValueRsp KVSetterProxy::SetValue(SetValueReq req)
{
    return CallRPC<SetValueRsp>(req);
}

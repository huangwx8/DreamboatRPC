#include <functional>

#include <apps/kv/KVClient.hh>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>

std::string KVGetterProxy::GetValue(GetValueArgs args)
{
    std::function<void(std::string)> cb = [=](std::string s) {
        log_dev("GetValue(%d) from server: %s\n", args.id(), s.c_str());
    };
    CallRPCAsync(cb, args);
    return {};
}

int KVSetterProxy::SetValue(SetValueArgs args)
{
    CallRPC(args);
    return 0;
}

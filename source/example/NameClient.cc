#include <functional>

#include <example/NameClient.hh>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>

std::string NameGetterProxy::GetName(GetNameArgs args)
{
    std::function<void(std::string)> cb = [=](std::string s) {
        log_dev("GetName(%d) from server: %s\n", args.id(), s.c_str());
    };
    CLIENT_CALL_RPC_Asynchronously(cb, args);
    return {};
}

int NameSetterProxy::SetName(SetNameArgs args)
{
    CLIENT_CALL_RPC(args);
    return 0;
}

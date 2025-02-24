#include <example/NameServer.hh>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>

RpcResult NameGetterImpl::Handle(const RpcMessage& Context)
{
    SERVER_EXEC_RPC(NameGetterImpl::GetName, GetNameArgs);
}

std::string NameGetterImpl::GetName(GetNameArgs args)
{
    auto ret = _base->GetNameInternal(args);
    log_dev("NameGetterImpl::GetName return %s\n", ret.c_str());
    return ret;
}

RpcResult NameSetterImpl::Handle(const RpcMessage& Context)
{
    SERVER_EXEC_RPC(NameSetterImpl::SetName, SetNameArgs);
}

int NameSetterImpl::SetName(SetNameArgs args)
{
    auto ret = _base->SetNameInternal(args);
    log_dev("NameSetterImpl::SetName return %d\n", ret);
    return ret;
}

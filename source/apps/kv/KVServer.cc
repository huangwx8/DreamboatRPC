#include <apps/kv/KVServer.hh>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>

RpcResult KVGetterImpl::Handle(const RpcMessage& Context)
{
    HandleRPC(KVGetterImpl::GetValue, GetValueArgs);
}

std::string KVGetterImpl::GetValue(GetValueArgs args)
{
    auto ret = _base->GetValueInternal(args);
    log_dev("KVGetterImpl::GetValue return %s\n", ret.c_str());
    return ret;
}

RpcResult KVSetterImpl::Handle(const RpcMessage& Context)
{
    HandleRPC(KVSetterImpl::SetValue, SetValueArgs);
}

int KVSetterImpl::SetValue(SetValueArgs args)
{
    auto ret = _base->SetValueInternal(args);
    log_dev("KVSetterImpl::SetValue return %d\n", ret);
    return ret;
}

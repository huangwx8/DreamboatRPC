#include <apps/kv/KVServer.hh>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>

RpcResult KVGetterImpl::Handle(const RpcMessage& Context)
{
    HandleRPC(GetValueReq, GetValueRsp, KVGetterImpl::GetValue);
}

GetValueRsp KVGetterImpl::GetValue(GetValueReq req)
{
    auto ret = _base->GetValueInternal(req);
    log_dev("KVGetterImpl::GetValue return %s\n", ret.name().c_str());
    return ret;
}

RpcResult KVSetterImpl::Handle(const RpcMessage& Context)
{
    HandleRPC(SetValueReq, SetValueRsp, KVSetterImpl::SetValue);
}

SetValueRsp KVSetterImpl::SetValue(SetValueReq req)
{
    auto ret = _base->SetValueInternal(req);
    log_dev("KVSetterImpl::SetValue return %d\n", ret.code());
    return ret;
}

#pragma once

#include <apps/kv/KVService.hh>

class KVGetterImpl : public KVGetterBase
{
public:
    KVGetterImpl(KVServiceBase* base): _base(base) {}
    virtual ~KVGetterImpl() = default;
    virtual RpcResult Handle(const RpcMessage& Context) override;
    virtual std::string GetValue(GetValueArgs args) override;
private:
    KVServiceBase* _base;
};

class KVSetterImpl : public KVSetterBase
{
public:
    KVSetterImpl(KVServiceBase* base): _base(base) {}
    virtual ~KVSetterImpl() = default;
    virtual RpcResult Handle(const RpcMessage& Context) override;
    virtual int SetValue(SetValueArgs args) override;
private:
    KVServiceBase* _base;
};

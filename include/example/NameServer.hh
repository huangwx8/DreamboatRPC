#pragma once

#include <example/NameService.hh>

class NameGetterImpl : public NameGetterBase
{
public:
    NameGetterImpl(NameServiceBase* base): _base(base) {}
    virtual ~NameGetterImpl() = default;
    virtual RpcResult Handle(const RpcMessage& Context) override;
    virtual std::string GetName(GetNameArgs args) override;
private:
    NameServiceBase* _base;
};

class NameSetterImpl : public NameSetterBase
{
public:
    NameSetterImpl(NameServiceBase* base): _base(base) {}
    virtual ~NameSetterImpl() = default;
    virtual RpcResult Handle(const RpcMessage& Context) override;
    virtual int SetName(SetNameArgs args) override;
private:
    NameServiceBase* _base;
};

#pragma once

#include <apps/kv/KVService.hh>

class KVGetterProxy : public KVGetterBase
{
public:
    KVGetterProxy() = default;
    virtual ~KVGetterProxy() = default;
    virtual std::string GetValue(GetValueArgs) override;
};

class KVSetterProxy : public KVSetterBase
{
public:
    KVSetterProxy() = default;
    virtual ~KVSetterProxy() = default;
    virtual int SetValue(SetValueArgs) override;
};

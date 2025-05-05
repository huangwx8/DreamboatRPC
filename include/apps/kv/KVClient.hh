#pragma once

#include <apps/kv/KVService.hh>

class KVGetterProxy : public KVGetterBase
{
public:
    KVGetterProxy() = default;
    virtual ~KVGetterProxy() = default;
    virtual GetValueRsp GetValue(GetValueReq) override;
};

class KVSetterProxy : public KVSetterBase
{
public:
    KVSetterProxy() = default;
    virtual ~KVSetterProxy() = default;
    virtual SetValueRsp SetValue(SetValueReq) override;
};

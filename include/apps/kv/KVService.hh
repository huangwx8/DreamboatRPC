#pragma once

#include <mutex>

#include <string>
#include <map>

#include <common/RpcTypes.hh>
#include <common/RpcServiceBase.hh>

#include <KVService.pb.h>

#define NUM_NAME_DICTIONARY_BUCKETS 5

class KVServiceBase
{
public:
    KVServiceBase() = default;
    ~KVServiceBase() = default;
    std::string GetValueInternal(GetValueArgs);
    int SetValueInternal(SetValueArgs);
private:
    std::map<int, std::string> dicts[NUM_NAME_DICTIONARY_BUCKETS];
    mutable std::mutex dict_locks[NUM_NAME_DICTIONARY_BUCKETS];
};

class KVGetterBase : public RpcServiceBase
{
public:
    KVGetterBase()
    {
        ServiceName = "KVGetter";
    }
    virtual ~KVGetterBase() = default;
    virtual std::string GetValue(GetValueArgs) = 0;
};

class KVSetterBase : public RpcServiceBase
{
public:
    KVSetterBase()
    {
        ServiceName = "KVSetter";
    }
    virtual ~KVSetterBase() = default;
    virtual int SetValue(SetValueArgs) = 0;
};

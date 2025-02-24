#pragma once

#include <mutex>

#include <string>
#include <map>

#include <common/RpcTypes.hh>
#include <common/RpcServiceBase.hh>

#include <NameService.pb.h>

#define NUM_NAME_DICTIONARY_BUCKETS 5

class NameServiceBase
{
public:
    NameServiceBase() = default;
    ~NameServiceBase() = default;
    std::string GetNameInternal(GetNameArgs);
    int SetNameInternal(SetNameArgs);
private:
    std::map<int, std::string> dicts[NUM_NAME_DICTIONARY_BUCKETS];
    mutable std::mutex dict_locks[NUM_NAME_DICTIONARY_BUCKETS];
};

class NameGetterBase : public RpcServiceBase
{
public:
    NameGetterBase()
    {
        ServiceName = "NameGetter";
    }
    virtual ~NameGetterBase() = default;
    virtual std::string GetName(GetNameArgs) = 0;
};

class NameSetterBase : public RpcServiceBase
{
public:
    NameSetterBase()
    {
        ServiceName = "NameSetter";
    }
    virtual ~NameSetterBase() = default;
    virtual int SetName(SetNameArgs) = 0;
};

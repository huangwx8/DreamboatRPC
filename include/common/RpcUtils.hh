#pragma once

#include <common/RpcTypes.hh>
#include <serialization/Serializer.hh>
#include <string.h>
#include <string>
#include <future>
#include <cassert>

using string = char[512];

void ParseParam(const char* In);

template<typename... ArgumentTypes>
void ParseParam(const char* In, const char* Type, void* Out, ArgumentTypes... Arguments)
{
    int Offset = Serializer::Deserialize(In, Out, Type);
    ParseParam(In + Offset, Arguments...);
}

template<typename T>
void ParseProtoStruct(const char* In, T& Out)
{
    char data[4096];

    ParseParam(In, "string", data);

    Out.ParseFromString(data);
}

int PackParam(char* Out);
int PackParam(char* Out, int In);
int PackParam(char* Out, float In);
int PackParam(char* Out, const char* In);
int PackParam(char* Out, std::string In);

template<typename T, typename... ArgumentTypes>
int PackParam(char* Out, T In, ArgumentTypes... Arguments)
{
    int Offset = PackParam(Out, In);
    return Offset + PackParam(Out + Offset, Arguments...);
}

template<typename T>
int PackProtoStruct(char* Out, T In)
{
    return PackParam(Out, In.SerializeAsString());
}

template<typename T>
RpcResult ToRpcResult(T output)
{
    RpcResult res;
    int size = PackProtoStruct(res.return_buffer, output);
    assert(size < MAX_RPC_RETURN_VALUE);
    return res;
}

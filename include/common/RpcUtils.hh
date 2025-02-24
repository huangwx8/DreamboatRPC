#pragma once

#include <common/RpcTypes.hh>
#include <serialization/Serializer.hh>
#include <string.h>
#include <string>

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

#define INIT_RPCMESSAGE()\
RpcMessage __RpcMessage;\
if (__RpcMessage.header.seqno == -1) return {};\
strcpy(__RpcMessage.header.servicename, GetServiceName());\

#define CLIENT_CALL_RPC(P)\
{\
    INIT_RPCMESSAGE()\
    __RpcMessage.header.body_length = PackProtoStruct(&(__RpcMessage.body.parameters[0]), P);\
    Invoke(__RpcMessage);\
}

#define CLIENT_CALL_RPC_Asynchronously(F, P)\
{\
    INIT_RPCMESSAGE()\
    __RpcMessage.header.body_length = PackProtoStruct(&(__RpcMessage.body.parameters[0]), P);\
    Invoke(__RpcMessage);\
}

RpcResult ToRpcResult(int i);

RpcResult ToRpcResult(float f);

RpcResult ToRpcResult(std::string s);

#define SERVER_EXEC_RPC(RpcImpl, T)\
{\
    T Arg;\
    ParseProtoStruct(&(Context.body.parameters[0]), Arg);\
    auto ret = RpcImpl(Arg);\
    return ToRpcResult(ret);\
}

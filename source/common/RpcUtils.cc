#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <common/RpcUtils.hh>


void ParseParam(const char* In) { }

int PackParam(char* Out) { return 0; }
int PackParam(char* Out, int In) { return Serializer::Serialize(&In, Out, "int"); }
int PackParam(char* Out, float In){ return Serializer::Serialize(&In, Out, "float"); }
int PackParam(char* Out, const char* In) { return Serializer::Serialize(In, Out, "string"); }
int PackParam(char* Out, std::string In) { return Serializer::Serialize(In.c_str(), Out, "string"); }

RpcResult ToRpcResult(int i) 
{
    RpcResult res;
    res.type = 1;
    PackParam(res.return_buffer, i);
    return res;
}

RpcResult ToRpcResult(float f) 
{
    RpcResult res;
    res.type = 2;
    PackParam(res.return_buffer, f);
    return res;
}

RpcResult ToRpcResult(std::string s) 
{
    RpcResult res;
    res.type = 3;
    const char* p = s.c_str();
    PackParam(res.return_buffer, p);
    return res;
}

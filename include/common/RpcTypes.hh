#pragma once

#include <strings.h>
#include <common/Guid.hh>

#define RPC_MAGIC_NUMBER 0xabcd
#define RPC_VERSION 1

#define MAX_RPC_NAME_SIZE 32
#define MAX_RPC_PARAMS_SIZE 480
#define MAX_RPC_RETURN_VALUE 480

#define INT_TYPE "int"
#define FLOAT_TYPE "float"
#define STRING_TYPE "string"

struct RpcHeader
{
    unsigned short magic;
    unsigned short version;
    bool need_return;
    int seqno;
    int body_length;
    char servicename[MAX_RPC_NAME_SIZE];
};

struct RpcBody
{
    char parameters[MAX_RPC_PARAMS_SIZE];
};

struct RpcMessage
{
    RpcMessage() 
    {
        header.magic = RPC_MAGIC_NUMBER;
        header.version = RPC_VERSION;
        header.need_return = false;
        header.seqno = 0;
        header.body_length = 0;
        bzero(&header.servicename, sizeof(header.servicename));
        bzero(&body, sizeof(body));
    }
    RpcHeader header;
    RpcBody body;
};

struct RpcResult
{
    RpcResult()
    {
        seqno = 0;
    }
    RpcResult(const RpcResult& other)
    {
        seqno = other.seqno;
    }
    int seqno;
    char return_buffer[MAX_RPC_RETURN_VALUE];
};

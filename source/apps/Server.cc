#include <string>
#include <cstdio>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>
#include <common/ArgsParser.hh>

#include <server/RpcServer.hh>

#include <apps/kv/KVServer.hh>

static RpcServer::Options GetOptions(int argc, char* argv[])
{
    const char* ip = parse_str(argc, argv, "-ip=");
    int port = parse_int(argc, argv, "-port=");
    const char* log_path = parse_str(argc, argv, "-log=");
    if (ip == nullptr)
    {
        ip = "localhost";
    }
    if (port == -1)
    {
        port = 8888;
    }
    if (log_path == nullptr)
    {
        log_path = "server.log";
    }
    return RpcServer::Options{.ip_addr = std::string(ip), .port = port, .log_path = "log/" + std::string(log_path)};
}

int main(int argc, char* argv[])
{
    KVServiceBase base;
    KVGetterImpl GetterImpl(&base);
    KVSetterImpl SetterImpl(&base);
    auto options = GetOptions(argc, argv);
    RpcServer ServerStub(options);
    
    // 实现绑定到RPC服务端
    ServerStub.RegisterService(&GetterImpl);
    ServerStub.RegisterService(&SetterImpl);

    // 启动RPC服务端
    ServerStub.Main(argc, argv);
}

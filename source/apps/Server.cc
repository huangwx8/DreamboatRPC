#include <string>
#include <cstdio>

#include <common/RpcUtils.hh>
#include <common/Logger.hh>
#include <common/ArgsParser.hh>

#include <server/RpcServer.hh>
#include <server/ServiceReporter.hh>

#include <apps/kv/KVServer.hh>

static RpcServer::Options GetOptions(int argc, char* argv[])
{
    const char* namesvr_ip = parse_str(argc, argv, "-namesvr_ip=");
    int namesvr_port = parse_int(argc, argv, "-namesvr_port=");
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
    return RpcServer::Options{
        .namesvr_ip_addr = std::string(namesvr_ip), 
        .namesvr_port = namesvr_port, 
        .ip_addr = std::string(ip), 
        .port = port, 
        .log_path = "log/" + std::string(log_path)
    };
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

    std::shared_ptr<ServiceReporter> reporter = ServiceReporter::GetServiceReporter(
        {.svr_ip=options.namesvr_ip_addr, .svr_port = options.namesvr_port});

    reporter->RegisterMyself(options.ip_addr, options.port, "KV");

    // 启动RPC服务端
    ServerStub.Main(argc, argv);
}

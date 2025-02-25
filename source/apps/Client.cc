#include <cstring>
#include <unistd.h>

#include <future>

#include <common/Logger.hh>
#include <common/ArgsParser.hh>

#include <client/RpcClient.hh>

#include <apps/kv/KVClient.hh>

#include <client/ServiceDiscoverer.hh>

static RpcClient::Options GetOptions(int argc, char* argv[])
{
    const char* callee = parse_str(argc, argv, "-callee=");
    const char* ip = parse_str(argc, argv, "-ip=");
    int port = parse_int(argc, argv, "-port=");
    if (ip == nullptr || port == -1)
    {
        printf("usage: %s -ip=svr_addr -port=port_number\n", basename(argv[0]));
        exit(1);
    }
    const char* log_path = parse_str(argc, argv, "-log=");

    return RpcClient::Options{.callee = std::string(callee), .svr_ip = std::string(ip), .svr_port = port, .log_path = log_path ? "log/" + std::string(log_path) : std::string()};
}

void crazy_read(std::shared_ptr<RpcClient> ClientStub)
{
    auto Getter = ClientStub->GetProxy<KVGetterProxy>();
    for (int i = 0; i < 100000; i += (std::rand() % 1000)) {
        log_dev("make a read request at %d\n", i);
        GetValueArgs args;
        args.set_id(i);
        Getter->GetValue(args);
    }
}

void crazy_write(std::shared_ptr<RpcClient> ClientStub)
{
    auto Setter = ClientStub->GetProxy<KVSetterProxy>();
    std::string data = "I can see Russia from my house!";
    for (int i = 0; i < 100000; i += (std::rand() % 1000)) {
        std::string code = data;
        for (char& ch : code) {
            if (ch >= 'a' && ch <= 'z') {
                ch = 'a' + (ch + i - 'a') % 26;
            }
        }
        log_dev("make a write request at %d with %s\n", i, code.c_str());
        SetValueArgs args;
        args.set_id(i);
        args.set_name(code);
        Setter->SetValue(args);
    }
}

int main(int argc, char* argv[])
{
    auto options = GetOptions(argc, argv);

    std::shared_ptr<ServiceDiscoverer> discovery = ServiceDiscoverer::GetServiceDiscoverer(
        {.svr_ip=options.svr_ip, .svr_port = options.svr_port});

    std::vector<ServiceDiscoverer::DiscoveredResult> results = discovery->RequestServiceList();

    bool foundSvr = false;

    for (ServiceDiscoverer::DiscoveredResult& result : results)
    {
        if (result.service_name == options.callee)
        {
            options.svr_ip = result.ip;
            options.svr_port = result.port;
            foundSvr = true;
            break;
        }
    }

    if (!foundSvr)
    {
        printf("Cannot find callee %s's address\n", options.callee.c_str());
        return 1;
    }

    auto&& ClientStub = RpcClient::GetRpcClient(options);

    auto fu1 = std::async(&crazy_read, ClientStub);
    auto fu2 = std::async(&crazy_write, ClientStub);

    fu1.wait();
    fu2.wait();

    sleep(10);

    return 0;
}

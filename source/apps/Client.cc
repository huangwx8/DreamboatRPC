#include <cstring>
#include <unistd.h>

#include <future>

#include <common/Logger.hh>
#include <common/ArgsParser.hh>

#include <client/RpcClient.hh>

#include <apps/kv/KVClient.hh>

#include <client/ClientFactory.hh>

static RpcClient::Options GetOptions(int argc, char* argv[])
{
    const char* ip = parse_str(argc, argv, "-ip=");
    int port = parse_int(argc, argv, "-port=");
    if (ip == nullptr || port == -1)
    {
        printf("usage: %s -ip=svr_addr -port=port_number\n", basename(argv[0]));
        exit(1);
    }

    return RpcClient::Options{.svr_ip = std::string(ip), .svr_port = port};
}

void crazy_read(std::shared_ptr<RpcClient> ClientStub)
{
    auto Getter = ClientStub->GetProxy<KVGetterProxy>();
    for (int i = 0; i < 10000; i += (std::rand() % 100)) {
        printf("make a read request at %d\n", i);
        GetValueReq req;
        req.set_id(i);
        Getter->GetValue(req);
    }
}

void crazy_write(std::shared_ptr<RpcClient> ClientStub)
{
    auto Setter = ClientStub->GetProxy<KVSetterProxy>();
    std::string data = "I can see Russia from my house!";
    for (int i = 0; i < 10000; i += (std::rand() % 100)) {
        std::string code = data;
        for (char& ch : code) {
            if (ch >= 'a' && ch <= 'z') {
                ch = 'a' + (ch + i - 'a') % 26;
            }
        }
        printf("make a write request at %d with %s\n", i, code.c_str());
        SetValueReq req;
        req.set_id(i);
        req.set_name(code);
        Setter->SetValue(req);
    }
}

int main(int argc, char* argv[])
{
    auto options = GetOptions(argc, argv);
    
    RpcClientFactory factory(options);

    auto&& getter = factory.MakeRpcClient("KVGetter");
    auto&& setter = factory.MakeRpcClient("KVSetter");

    auto fu1 = std::async(&crazy_read, getter);
    auto fu2 = std::async(&crazy_write, setter);

    fu1.wait();
    fu2.wait();

    sleep(10);

    return 0;
}

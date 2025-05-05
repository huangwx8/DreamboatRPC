#include <client/ClientFactory.hh>
#include <iostream>
#include <client/ServiceDiscoverer.hh>

bool DiscoverServer(const RpcClient::Options& options_ns_, RpcClient::Options& options_rpc)
{
    auto discovery = ServiceDiscoverer::GetServiceDiscoverer({options_ns_.svr_ip, options_ns_.svr_port});
    auto results = discovery->RequestServiceList(options_ns_.callee);

    if (results.size() > 0)
    {
        options_rpc.svr_ip = results[0].ip;
        options_rpc.svr_port = results[0].port;
        return true;
    }

    return false;
}


std::shared_ptr<RpcClient> RpcClientFactory::MakeRpcClient(const std::string& callee_service_name)
{
    options_ns.callee = callee_service_name;
    RpcClient::Options options_rpc;

    if (!DiscoverServer(options_ns, options_rpc))
    {
        std::cerr << "Cannot find callee service: " << callee_service_name << std::endl;
        return nullptr;
    }

    return RpcClient::GetRpcClient(options_rpc);
}

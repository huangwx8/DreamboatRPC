#pragma once

#include <string>
#include <client/RpcClient.hh>


class RpcClientFactory
{
    using Options = RpcClient::Options;
public:
    RpcClientFactory(const Options& options) : options_ns(options) {}

    // Method to create RpcClient based on the service discovery
    std::shared_ptr<RpcClient> MakeRpcClient(const std::string& callee_service_name);
private:

    // server info of name server
    Options options_ns;
};

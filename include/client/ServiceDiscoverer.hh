#pragma once

#include <atomic>
#include <mutex>
#include <functional>
#include <string>
#include <condition_variable>

class ServiceDiscoverer
{

public:

    struct Options
    {
        std::string svr_ip;
        int svr_port;
    };

    struct DiscoveredResult
    {
        std::string ip;
        int port;
        std::string service_name;
    };

public:

    ServiceDiscoverer(Options options);
    ~ServiceDiscoverer();
    ServiceDiscoverer(ServiceDiscoverer&) = delete;
    ServiceDiscoverer(ServiceDiscoverer&&) = delete;

    /** 
     * Construct and initialize
     */
    static std::shared_ptr<ServiceDiscoverer> GetServiceDiscoverer(Options options);

    /** 
     * Request a list of running services from name service
     */
    std::vector<DiscoveredResult> RequestServiceList();

private:

    Options _options;

};

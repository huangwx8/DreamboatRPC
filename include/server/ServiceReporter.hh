#pragma once

#include <atomic>
#include <mutex>
#include <functional>
#include <string>
#include <condition_variable>

class ServiceReporter
{

public:

    struct Options
    {
        std::string svr_ip;
        int svr_port;
    };

public:

    ServiceReporter(Options options);
    ~ServiceReporter();
    ServiceReporter(ServiceReporter&) = delete;
    ServiceReporter(ServiceReporter&&) = delete;

    /** 
     * Construct and initialize
     */
    static std::shared_ptr<ServiceReporter> GetServiceReporter(Options options);

    /** 
     * Register myself as an available service
     */
    bool RegisterMyself(std::string ip, int port, std::string service_name);

private:

    Options _options;

};

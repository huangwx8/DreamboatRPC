#include <string.h>
#include <thread>
#include <curl/curl.h>
#include <json/json.h>

#include <client/ServiceDiscoverer.hh>


ServiceDiscoverer::ServiceDiscoverer(Options options):
    _options(options)
{
}

ServiceDiscoverer::~ServiceDiscoverer()
{
}

std::shared_ptr<ServiceDiscoverer> ServiceDiscoverer::GetServiceDiscoverer(Options options)
{
    std::shared_ptr<ServiceDiscoverer> sp = std::make_shared<ServiceDiscoverer>(options);

    return sp; 
}

static size_t cb(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

std::vector<ServiceDiscoverer::DiscoveredResult> ServiceDiscoverer::RequestServiceList(std::string callee)
{
    std::vector<ServiceDiscoverer::DiscoveredResult> results;

    CURL* curl = curl_easy_init();
    
    if (!curl) {
        return results;
    }

    char buffer[200];  // Increased buffer size to accommodate the callee parameter
    snprintf(buffer, sizeof(buffer), "http://%s:%d/discover?callee=%s", _options.svr_ip.c_str(), _options.svr_port, callee.c_str());
    
    std::string url(buffer);

    // Set the URL and HTTP method
    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);  // Use HTTP GET for querying
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        return results;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        return results;
    }

    // Parse the JSON response
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;

    bool success = reader->parse(
        response_data.c_str(),                      // Start of JSON string
        response_data.c_str() + response_data.size(),   // End of JSON string
        &root,                                   // Root object to store parsed data
        &errors                                 // Store any parsing errors
    );

    if (!success) {
        return results;
    }

    // Check if the root is an array
    if (!root.isArray()) {
        return results;
    }

    // Iterate over the array
    for (const auto& element : root) {
        // Access values in each object
        std::string ip = element["ip"].asString();
        int port = element["port"].asInt();
        
        // Save the values
        results.emplace_back(ServiceDiscoverer::DiscoveredResult{.ip=ip, .port=port});
    }

    // Clean up cURL
    curl_easy_cleanup(curl);

    return results;
}

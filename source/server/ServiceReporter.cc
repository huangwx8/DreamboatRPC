#include <string.h>
#include <thread>
#include <curl/curl.h>
#include <json/json.h>

#include <server/ServiceReporter.hh>


ServiceReporter::ServiceReporter(Options options):
    _options(options)
{
}

ServiceReporter::~ServiceReporter()
{
}

std::shared_ptr<ServiceReporter> ServiceReporter::GetServiceReporter(Options options)
{
    std::shared_ptr<ServiceReporter> sp = std::make_shared<ServiceReporter>(options);

    return sp; 
}

static size_t cb(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

bool ServiceReporter::RegisterMyself(std::string ip, int port, std::string service_name)
{
    CURL* curl = curl_easy_init();
    
    if (!curl) {
        return false;
    }

    // JSON data to send in the POST request
    Json::Value json_data;
    json_data["ip"] = ip;
    json_data["port"] = port;
    json_data["service_name"] = service_name;

    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, json_data);

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "http://%s:%d/register", _options.svr_ip.c_str(), _options.svr_port);
    
    std::string url(buffer);

    // Create a curl_slist for custom headers
    struct curl_slist* headers = NULL;

    // Add custom headers
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Set the URL and POST data
    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Python server URL
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        return false;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        return false;
    }

    // Clean up cURL
    curl_easy_cleanup(curl);

    return true;
}

bool ServiceReporter::SendHeartbeat(std::string ip, int port)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        return false;
    }

    // Prepare empty JSON data for heartbeat
    Json::Value json_data;
    json_data["ip"] = ip;
    json_data["port"] = port;
    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, json_data);

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "http://%s:%d/heartbeat", _options.svr_ip.c_str(), _options.svr_port);

    std::string url(buffer);

    // Create a curl_slist for custom headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Set the URL and POST data
    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Server URL
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        return false;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200)
    {
        curl_easy_cleanup(curl);
        return false;
    }

    // Clean up cURL
    curl_easy_cleanup(curl);

    return true;
}

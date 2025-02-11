#ifndef REPORT_HPP
#define REPORT_HPP

#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include <sys/sysinfo.h>

class Report
{
private:
    std::string server_url;
    bool verbose;
    CURL *curl_session;
    std::string user_agent;
    struct curl_slist *headers = NULL;
    std::string report_payload_str;

public:
    Report(const std::string &url, bool is_verbose);

    ~Report();

    void send();
};

#endif // REPORT_HPP

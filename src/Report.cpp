/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */

#include <cstring>
#include <stdexcept>
#include <system_error>
#include <cerrno>
#include <curl/curl.h>
#include <unistd.h>

#include "Report.hpp"
#include "log_utils.h"

using namespace std;

static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
    // For now, we just discard the output to suppress printing to stdout.
    // Return number of bytes handled.
    return size * nmemb;
}

static string statsToPayload(const string &hostname, struct sysinfo *info)
{
    return "{\"hostname\":\"" + hostname + "\",\"uptime\":" + to_string(info->uptime) + "}";
}

Report::Report(const string &url, bool is_verbose = false)
    : server_url(url), verbose(is_verbose), curl_session(curl_easy_init())
{
    if (!curl_session)
    {
        throw runtime_error("Failed to initialize CURL");
    }
    curl_easy_setopt(curl_session, CURLOPT_CAINFO, "curl-ca-bundle.crt");
    user_agent = "libcurl/" + string(curl_version_info(CURLVERSION_NOW)->version);
    curl_easy_setopt(curl_session, CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(curl_session, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl_session, CURLOPT_URL, server_url.c_str());
    curl_easy_setopt(curl_session, CURLOPT_TIMEOUT_MS, 5000);
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDS, report_payload_str.c_str());
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl_session, CURLOPT_HTTPHEADER, headers);
}

Report::~Report()
{
    if (headers)
    {
        curl_slist_free_all(headers);
    }
    if (curl_session)
    {
        curl_easy_cleanup(curl_session);
    }
}

// Function to send the system info (hostname and uptime)
void Report::send()
{

    struct sysinfo sys_info;
    if (sysinfo(&sys_info) != 0)
    {
        int err = errno;
        throw system_error(err, system_category(), "Failed to retrieve system info: " + string(strerror(err)));
    }
    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        int err = errno;
        throw system_error(err, system_category(), "Failed to retrieve hostname: " + string(strerror(err)));
    }

    // Prepare the POST data
    report_payload_str = statsToPayload(hostname, &sys_info);
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDS, report_payload_str.c_str());
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDSIZE, report_payload_str.size());

    CURLcode res = curl_easy_perform(curl_session);
    if (res != CURLE_OK)
    {
        throw runtime_error(curl_easy_strerror(res));
    }

    // check if the response code is 201 (Created), else throw error
    long response_code;
    curl_easy_getinfo(curl_session, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 201)
    {
        throw runtime_error("Could not send data to '" + server_url +
                            "', HTTP response code=" + to_string(response_code));
    }

    if (verbose)
    {
        OD_LOG_INFO("Sent system info (http_code=%ld): %s",
                 response_code, report_payload_str.c_str());
    }
}

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

/**
 * @brief curl write callback used in order to suppress printing to stdout
 */
static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
    // return number of bytes handled.
    return size * nmemb;
}

/**
 * @brief Constructs a Report object with the specified server URL.
 * @param url The server URL to send data to.
 * @throws std::runtime_error if cURL initialization fails.
 */
Report::Report(const string &url)
    : server_url(url), curl_session(curl_easy_init()), headers(nullptr)
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
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl_session, CURLOPT_HTTPHEADER, headers);
}

/**
 * @brief Destroys the Report object, cleaning up resources.
 */
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

/**
 * @brief Sends the specified JSON payload to the server.
 * @param jsonStrPayload The JSON string to send.
 * @throws std::runtime_error if sending the data fails.
 */
void Report::send(const string &jsonStrPayload)
{
    // prepare the POST data
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDS, jsonStrPayload.c_str());
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDSIZE, jsonStrPayload.size());

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
        throw runtime_error("Got unexpected HTTP response code '" +
                            to_string(response_code) + "' from '" + server_url + "'");
    }

    OD_LOG_INFO("Sent system info (http_code=%ld).", response_code);
    OD_LOG_DBG("Sent data: %s", jsonStrPayload.c_str());
}

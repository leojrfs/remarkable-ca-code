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
#include <optional>

#include "HTTPClient.hpp"
#include "log_utils.h"

using namespace std;
using namespace ob;

/**
 * @brief curl write callback used in order to suppress printing to stdout
 */
static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
    // return number of bytes handled.
    return size * nmemb;
}

/**
 * @brief Constructs a HTTPClient object with the specified server URL.
 * @param url The server URL to send data to.
 * @throws std::runtime_error if cURL initialization fails.
 */
HTTPClient::HTTPClient(const string &url)
    : server_url(url), curl_session(curl_easy_init()), headers(nullptr)
{
    if (!curl_session)
    {
        throw runtime_error("Failed to initialize CURL");
    }
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
 * @brief Destroys the HTTPClient object, cleaning up resources.
 */
HTTPClient::~HTTPClient()
{
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_session);
}

/**
 * @brief Sends a POST request with the specified payload to the server.
 *
 * Sets up the POST data and performs the request. Checks for successful
 * completion and validates the HTTP response code.
 *
 * @param payload The JSON string to send.
 * @return std::optional<http_client_error> An optional error code; empty if successful.
 */
optional<http_client_error> HTTPClient::post(const string &payload)
{
    // prepare the POST data
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl_session, CURLOPT_POSTFIELDSIZE, payload.size());

    OD_LOG_DBG("Executing POST request to '%s'.", server_url.c_str());
    OD_LOG_DBG("POST payload='%s'", payload.c_str());

    CURLcode res = curl_easy_perform(curl_session);
    if (res != CURLE_OK)
    {
        OD_LOG_ERR("Failed while performing POST request: %s", curl_easy_strerror(res));
        return http_client_error::request_failed;
    }

    // check if the response code is 201 (Created), else throw error
    long response_code;
    curl_easy_getinfo(curl_session, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 201)
    {
        OD_LOG_ERR("Got unexpected HTTP response code '%ld' from '%s'.",
                   response_code, server_url.c_str());
        return http_client_error::unexpected_http_response_code;
    }

    OD_LOG_INFO("POST request sucessful (http_code=%ld).", response_code);

    return {};
}

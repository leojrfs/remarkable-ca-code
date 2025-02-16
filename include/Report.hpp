/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#ifndef REPORT_HPP
#define REPORT_HPP

#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include <sys/sysinfo.h>

/**
 * @class Report
 * @brief Manages the sending of system information to a specified server.
 */
class Report
{
private:
    std::string server_url;               /**< The server URL to send data to. */
    CURL *curl_session = nullptr;         /**< cURL session handle. */
    std::string user_agent;               /**< User agent string for the cURL session. */
    struct curl_slist *headers = nullptr; /**< List of HTTP headers for the cURL session. */

public:
    /**
     * @brief Constructs a Report object with the specified server URL.
     * @param url The server URL to send data to.
     * @throws std::runtime_error if cURL initialization fails.
     */
    Report(const std::string &url);

    /**
     * @brief Destroys the Report object, cleaning up resources.
     */
    ~Report();

    /**
     * @brief Sends the specified JSON payload to the server.
     * @param jsonStrPayload The JSON string to send.
     * @throws std::runtime_error if sending the data fails.
     */
    void send(const std::string &jsonStrPayload);
};

#endif // REPORT_HPP

/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#ifndef OBHTTPCLIENT_HPP
#define OBHTTPCLIENT_HPP

#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include <sys/sysinfo.h>
#include <optional>

namespace ob
{
    /**
     * @enum http_client_error
     * @brief Enumerates possible errors for the HTTPClient class.
     */
    enum class http_client_error
    {
        request_failed,                   /**< The HTTP request failed. */
        unexpected_http_response_code     /**< Received an unexpected HTTP response code. */
    };

    /**
     * @class HTTPClient
     * @brief Manages the sending of system information to a specified server.
     */
    class HTTPClient
    {
    private:
        std::string server_url;               /**< The server URL to send data to. */
        CURL *curl_session = nullptr;         /**< cURL session handle. */
        std::string user_agent;               /**< User agent string for the cURL session. */
        struct curl_slist *headers = nullptr; /**< List of HTTP headers for the cURL session. */

    public:
        /**
         * @brief Constructs a HTTPClient object with the specified server URL.
         * @param url The server URL to send data to.
         * @throws std::runtime_error if cURL initialization fails.
         */
        HTTPClient(const std::string &url);

        /**
         * @brief Destroys the HTTPClient object, cleaning up resources.
         */
        ~HTTPClient();

        /**
         * @brief Sends a POST request with the specified payload to the server.
         *
         * Sets up the POST data and performs the request. Checks for successful
         * completion and validates the HTTP response code.
         *
         * @param payload The JSON string to send.
         * @return std::optional<http_client_error> An optional error code; empty if successful.
         */
        std::optional<http_client_error> post(const std::string &payload);
    };
}
#endif // OBHTTPCLIENT_HPP

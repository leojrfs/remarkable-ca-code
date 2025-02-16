/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */

#include <string>
#include <atomic>
#include <stdexcept>
#include <getopt.h>
#include <signal.h>
#include "log_utils.h"
#include "init_utils.h"

#include "SystemInfo.hpp"
#include "HTTPClient.hpp"

using namespace std;

atomic<bool> running(true);
string server_url;
int interval_s;

uint8_t verbosity = LOG_VERBOSITY_DEFAULT;

static int parse_cmdline_arguments(int argc, char *argv[])
{
    bool arg_server_url_set = false;
    bool arg_interval_set = false;

    struct option long_options[] = {
        {"verbosity", required_argument, nullptr, 'v'},
        {"server-url", required_argument, nullptr, 's'},
        {"interval", required_argument, nullptr, 'i'},
        {nullptr, 0, nullptr, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "v:s:i:", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'v':
            verbosity = atoi(optarg);
            if ((verbosity < LOG_VERBOSITY_MIN) || (verbosity > LOG_VERBOSITY_MAX))
            {
                OD_LOG_ERR("verbosity value should be between '%d' and '%d'.\n",
                           LOG_VERBOSITY_MIN, LOG_VERBOSITY_MAX);
                return 1;
            }
            break;
        case 's':
            server_url = optarg;
            arg_server_url_set = true;
            break;
        case 'i':
            try
            {
                interval_s = std::stoi(optarg);
                if (interval_s < 1)
                {
                    OD_LOG_ERR("interval must be >= 1 second");
                    return 1;
                }
            }
            catch (const invalid_argument &)
            {
                OD_LOG_ERR("Invalid value for --interval: '%s'", optarg);
                return 1;
            }
            catch (const out_of_range &)
            {
                OD_LOG_ERR("Interval value out of range");
                return 1;
            }
            arg_interval_set = true;
            break;
        case '?':
            return 1;
        default:
            return 1;
        }
    }

    if (!arg_interval_set)
    {
        OD_LOG_STDERR("%s: argument '-i/--interval' is required", argv[0]);
        return 1;
    }

    if (!arg_server_url_set)
    {
        OD_LOG_STDERR("%s: argument '-s/--server-url' is required", argv[0]);
        return 1;
    }

    return 0;
}

void signal_handle_cb(int signum)
{
    if (signum == SIGTERM || signum == SIGINT)
    {
        OD_LOG_WARNING("Received termination signal. Stopping daemon...");
        running = false;
    }
    else if (signum == SIGHUP)
    {
        OD_LOG_WARNING("Received SIGHUP, but no action implemented.");
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;

    ret = parse_cmdline_arguments(argc, argv);
    if (ret)
    {
        OD_LOG_STDERR("Usage: %s [-v/--verbose] -s/--server-url <URL> -i/--interval <seconds>", argv[0]);
        // https://refspecs.linuxbase.org/LSB_3.1.1/LSB-Core-generic/LSB-Core-generic/iniscrptact.html
        // return 2 for invalid or excess argument(s)
        ret = 2;
        INIT_NOTIFY_FAILED_TO_STARTUP(ret);
        return ret;
    }

    // setup signal handlers
    struct sigaction sa{};
    sa.sa_handler = signal_handle_cb;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);

    // notify systemd that the daemon is ready
    INIT_NOTIFY_READY();
    try
    {
        ob::HTTPClient http_client(server_url);
        ob::SystemInfo systeminfo;

        while (running)
        {

            auto si_error = systeminfo.readSysInfo();
            if (si_error.has_value())
            {
                switch (si_error.value())
                {
                case (ob::SystemInfo::sysstats_error::failed_to_get_hostname):
                    OD_LOG_ERR("Failed to get hostname!");
                    break;
                case (ob::SystemInfo::sysstats_error::failed_to_get_sysinfo):
                    OD_LOG_ERR("Failed to get sysinfo!");
                    break;
                case (ob::SystemInfo::sysstats_error::failed_to_get_disk_stats):
                    OD_LOG_ERR("Failed to get disk stats!");
                    break;
                case (ob::SystemInfo::sysstats_error::failed_to_parse_meminfo):
                    OD_LOG_ERR("Failed to parse meminfo!");
                    break;
                default:
                    OD_LOG_ERR("Other sysstats error!");
                    break;
                }
            }

            auto tojson_result = systeminfo.toJson();
            if (!tojson_result.has_value())
            {
                switch (tojson_result.error())
                {
                case ob::SystemInfo::json_error::json_object_creation_error:
                    OD_LOG_ERR("Failed when creating JSON object!");
                    break;
                default:
                    OD_LOG_ERR("Other json_error error!");
                    break;
                }
            }

            string payload = tojson_result.value();
            OD_LOG_DBG("Executing POST request to '%s'.", server_url.c_str());
            OD_LOG_DBG("POST payload='%s'", payload.c_str());

            auto post_error = http_client.post(payload);
            if (post_error.has_value())
            {
                switch (post_error.value())
                {
                case ob::HTTPClient::error::request_failed:
                    OD_LOG_ERR("HTTP request failed!");
                    break;
                case ob::HTTPClient::error::unexpected_http_response_code:
                    OD_LOG_ERR("unexpected HTTP response code!");
                    break;
                default:
                    OD_LOG_ERR("Other HTTPClient error");
                    break;
                }
            }
            else
            {
                OD_LOG_INFO("POST request sucessful.");
            }
            sleep(interval_s);
            // kick watchdog
            INIT_NOTIFY_WATCHDOG();
        }
    }
    catch (const runtime_error &e)
    {
        OD_LOG_ERR("Shutting down daemon due to: %s", e.what());
        // return 1 for generic or unspecified error as specified in LSB docs
        ret = 1;
    }

    // notify daemon is stopping
    INIT_NOTIFY_STOPPING();

    OD_LOG_INFO("Demon has been sucessfully stopped.");

    return ret;
}
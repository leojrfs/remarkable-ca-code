/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */

#include <string>
#include <atomic>
#include <stdexcept>
#include <system_error>
#include <getopt.h>
#include <signal.h>
#include "log_utils.h"
#include "init_utils.h"
#include "Report.hpp"

using namespace std;

atomic<bool> running(true);
string server_url;
int interval_s;
bool verbose = false;

static int parse_cmdline_arguments(int argc, char *argv[])
{
    bool arg_server_url_set = false;
    bool arg_interval_set = false;

    struct option long_options[] = {
        {"verbose", no_argument, nullptr, 'v'},
        {"server-url", required_argument, nullptr, 's'},
        {"interval", required_argument, nullptr, 'i'},
        {nullptr, 0, nullptr, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "vs:i:", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'v':
            verbose = true;
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

    Report report(server_url, verbose);

    while (running)
    {
        try
        {
            report.send();
        }
        catch (const system_error &e)
        {
            OD_LOG_ERR("Shutting down daemon due to: %s", e.what());
            // return 1 for generic or unspecified error as specified in LSB docs
            ret = 1;
            break;
        }
        catch (const exception &e)
        {
            OD_LOG_ERR("Error while sending report: %s", e.what());
        }
        sleep(interval_s);
        // kick watchdog
        INIT_NOTIFY_WATCHDOG();
    }

    // notify daemon is stopping
    INIT_NOTIFY_STOPPING();

    OD_LOG_INFO("Demon has been sucessfully stopped.");

    return ret;
}
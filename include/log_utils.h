/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <cstdio>
#include <cstdint>

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>
#endif

#define LOG_VERBOSITY_ERROR 1
#define LOG_VERBOSITY_WARN 2
#define LOG_VERBOSITY_INFO 3
#define LOG_VERBOSITY_DBG 4
#define LOG_VERBOSITY_DEFAULT LOG_VERBOSITY_INFO
#define LOG_VERBOSITY_MIN LOG_VERBOSITY_ERROR
#define LOG_VERBOSITY_MAX LOG_VERBOSITY_DBG

#define PRINTF_NO_COLOUR "\033[0m"
#define PRINTF_RED "\033[0;31m"
#define PRINTF_YELLOW "\033[0;33m"
#define PRINTF_BLUE "\033[0;34m"
#define PRINTF_CYAN "\033[0;36m"

extern uint8_t verbosity;

#ifdef USE_SYSTEMD
#define OD_LOG_INFO(msg, ...)                               \
    do                                                      \
    {                                                       \
        if (verbosity >= LOG_VERBOSITY_INFO)                \
            sd_journal_print(LOG_INFO, msg, ##__VA_ARGS__); \
    } while (0)

#define OD_LOG_WARNING(msg, ...)                               \
    do                                                         \
    {                                                          \
        if (verbosity >= LOG_VERBOSITY_WARN)                   \
            sd_journal_print(LOG_WARNING, msg, ##__VA_ARGS__); \
    } while (0)

#define OD_LOG_DBG(msg, ...)                                 \
    do                                                       \
    {                                                        \
        if (verbosity >= LOG_VERBOSITY_DBG)                  \
            sd_journal_print(LOG_DEBUG, msg, ##__VA_ARGS__); \
    } while (0)

#define OD_LOG_ERR(msg, ...)                           \
    do                                                 \
    {                                                  \
        sd_journal_print(LOG_ERR, msg, ##__VA_ARGS__); \
    } while (0)

#else
#define OD_LOG_INFO(msg, ...)                                                                   \
    do                                                                                          \
    {                                                                                           \
        if (verbosity >= LOG_VERBOSITY_INFO)                                                    \
        {                                                                                       \
            fprintf(stdout, "[" PRINTF_BLUE "I" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
            fflush(stdout);                                                                     \
        }                                                                                       \
    } while (0)

#define OD_LOG_WARNING(msg, ...)                                                                  \
    do                                                                                            \
    {                                                                                             \
        if (verbosity >= LOG_VERBOSITY_WARN)                                                      \
        {                                                                                         \
            fprintf(stderr, "[" PRINTF_YELLOW "W" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
            fflush(stderr);                                                                       \
        }                                                                                         \
    } while (0)

#define OD_LOG_DBG(msg, ...)                                                                    \
    do                                                                                          \
    {                                                                                           \
        if (verbosity >= LOG_VERBOSITY_DBG)                                                     \
        {                                                                                       \
            fprintf(stdout, "[" PRINTF_CYAN "D" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
            fflush(stdout);                                                                     \
        }                                                                                       \
    } while (0)

#define OD_LOG_ERR(msg, ...)                                                               \
    do                                                                                     \
    {                                                                                      \
        fprintf(stderr, "[" PRINTF_RED "E" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
        fflush(stderr);                                                                    \
    } while (0)

#endif // USE_SYSTEMD

#define OD_LOG_STDERR(msg, ...)                   \
    do                                            \
    {                                             \
        fprintf(stderr, msg "\n", ##__VA_ARGS__); \
        fflush(stderr);                           \
    } while (0)

#endif // LOG_UTILS_H

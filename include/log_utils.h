#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <cstdio>
#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>
#endif

#ifdef USE_SYSTEMD
#define OD_LOG_INFO(msg, ...) sd_journal_print(LOG_INFO, msg, ##__VA_ARGS__)
#define OD_LOG_ERR(msg, ...) sd_journal_print(LOG_ERR, msg, ##__VA_ARGS__)
#define OD_LOG_ERR_RAW(msg, ...) sd_journal_print(LOG_ERR, msg, ##__VA_ARGS__)
#define OD_LOG_WARNING(msg, ...) sd_journal_print(LOG_WARNING, msg, ##__VA_ARGS__)
#else
#define PRINTF_NO_COLOUR "\033[0m"
#define PRINTF_RED "\033[0;31m"
#define PRINTF_YELLOW "\033[0;33m"
#define PRINTF_BLUE "\033[0;34m"
#define OD_LOG_INFO(msg, ...)                                                                   \
    {                                                                                        \
        fprintf(stdout, "[" PRINTF_BLUE "I" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
        fflush(stdout);                                                                      \
    }
#define OD_LOG_ERR(msg, ...)                                                                   \
    {                                                                                       \
        fprintf(stderr, "[" PRINTF_RED "E" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
        fflush(stderr);                                                                     \
    }
#define OD_LOG_ERR_RAW(msg, ...)                     \
    {                                             \
        fprintf(stderr, msg "\n", ##__VA_ARGS__); \
        fflush(stderr);                           \
    }
#define OD_LOG_WARNING(msg, ...)                                                                  \
    {                                                                                          \
        fprintf(stderr, "[" PRINTF_YELLOW "W" PRINTF_NO_COLOUR "] " msg "\n", ##__VA_ARGS__); \
        fflush(stderr);                                                                        \
    }
#endif

#endif // LOG_UTILS_H
/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include <errno.h>
#include <sys/sysinfo.h>

#include "sys_utils.h"

/**
 * @brief Parses memory information from the /proc/meminfo file.
 *
 *  Inspired by busybox's free.c
 *
 * @param[out] cached_kb Pointer to a variable where the Cached memory value (in KB) will be stored.
 * @param[out] available_kb Pointer to a variable where the MemAvailable value (in KB) will be stored.
 * @param[out] reclaimable_kb Pointer to a variable where the SReclaimable memory value (in KB) will be stored.
 *
 * @return 0 if the function was successful in parsing the required memory values,
 *         -errno if it failed to open `/proc/meminfo` or encountered an error during file operations,
 *         or 1 if the required fields (`Cached`, `MemAvailable`, `SReclaimable`) were not found in the file.
 *
 * @note The function expects the `/proc/meminfo` file to contain the `Cached`, `MemAvailable`,
 *       and `SReclaimable` fields. The function returns 1 if any of these fields are missing
 *       or the file cannot be opened.
 *
 * @see errno
 */
unsigned int parse_meminfo(unsigned long *cached_kb, unsigned long *available_kb, unsigned long *reclaimable_kb)
{
    char buf[60];
    FILE *fp;
    int seen_cached_and_available_and_reclaimable;
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        return -errno;
    }

    *cached_kb = *available_kb = 0;
    seen_cached_and_available_and_reclaimable = 3;
    while (fgets(buf, sizeof(buf), fp))
    {
        if (sscanf(buf, "Cached: %lu %*s\n", cached_kb) == 1)
            if (--seen_cached_and_available_and_reclaimable == 0)
                break;
        if (sscanf(buf, "MemAvailable: %lu %*s\n", available_kb) == 1)
            if (--seen_cached_and_available_and_reclaimable == 0)
                break;
        if (sscanf(buf, "SReclaimable: %lu %*s\n", reclaimable_kb) == 1)
            if (--seen_cached_and_available_and_reclaimable == 0)
                break;
    }

    /* Have to close because of NOFORK */
    fclose(fp);
    return seen_cached_and_available_and_reclaimable != 0;
}

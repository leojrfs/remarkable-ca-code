/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#include "SystemInfo.hpp"
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <json-c/json.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "sys_utils.h"

using namespace std;

/**
 * @brief Constructs a new SystemInfoException object.
 * 
 * @param message The error message.
 */
SystemInfoException::SystemInfoException(const string &message)
    : runtime_error(message) {}

/**
 * @brief Retrieves the system hostname.
 * 
 * @return The hostname as a string.
 * @throws SystemInfoException if hostname retrieval fails.
 */
static string getHostname()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        return string(hostname);
    }
    throw SystemInfoException("Failed to execute gethostname(). errno=" + to_string(errno));
}

/**
 * @brief Retrieves memory statistics as a JSON object.
 * 
 * Shows sizes in KiB just like `free`.
 * 
 * @param info Pointer to a valid sysinfo structure.
 * @return JSON object containing memory stats.
 * @throws SystemInfoException if retrieval fails.
 */
static json_object *getMemoryStatsJsonObj(struct sysinfo *info)
{
    json_object *memJson = json_object_new_object();
    if (!memJson)
        throw SystemInfoException("Failed to allocate JSON object for memory stats");

    unsigned long total_memory = (info->totalram * info->mem_unit) / 1024;
    unsigned long free_memory = (info->freeram * info->mem_unit) / 1024;
    unsigned long shared_memory = info->sharedram / 1024;
    unsigned long cached_kb, available_kb, reclaimable_kb;
    unsigned long long cached, cached_plus_free, available;

    int ret = parse_meminfo(&cached_kb, &available_kb, &reclaimable_kb);
    if (ret)
    {
        json_object_put(memJson);
        throw SystemInfoException("Failed to read '/proc/meminfo'. ret=" + to_string(ret));
    }

    available = ((unsigned long long)available_kb * 1024) / info->mem_unit;
    cached = ((unsigned long long)cached_kb * 1024) / info->mem_unit;
    cached += info->bufferram;
    cached += ((unsigned long long)reclaimable_kb * 1024) / info->mem_unit;
    cached_plus_free = cached + info->freeram;
    unsigned long long used_memory = (info->totalram - cached_plus_free) / 1024;

    json_object_object_add(memJson, "total_memory", json_object_new_int64(total_memory));
    json_object_object_add(memJson, "used_memory", json_object_new_int64(used_memory));
    json_object_object_add(memJson, "free_memory", json_object_new_int64(free_memory));
    json_object_object_add(memJson, "shared", json_object_new_int64(shared_memory));
    json_object_object_add(memJson, "cached", json_object_new_int64(cached / 1024));
    json_object_object_add(memJson, "available_memory", json_object_new_int64(available / 1024));

    return memJson;
}

/**
 * @brief Retrieves disk statistics as a JSON object.
 * 
 * Shows sizes in KiB just like `df` and uses disk mounted at `/`.
 * 
 * @return JSON object containing disk stats.
 * @throws SystemInfoException if retrieval fails.
 */
static json_object *getDiskStatsJsonObj()
{
    struct statfs s;
    if (statfs("/", &s) == 0)
    {
        json_object *diskJson = json_object_new_object();
        if (!diskJson)
            throw SystemInfoException("Failed to allocate JSON object for disk stats");

        long block_size = s.f_frsize;
        long total = s.f_blocks * block_size / 1024;
        long free = s.f_bfree * block_size / 1024;
        long available = s.f_bavail * block_size / 1024;
        long used = total - free;
        double usage_percent = (used * 100.0) / total;

        json_object_object_add(diskJson, "total_disk", json_object_new_int64(total));
        json_object_object_add(diskJson, "free_disk", json_object_new_int64(free));
        json_object_object_add(diskJson, "used_disk", json_object_new_int64(used));
        json_object_object_add(diskJson, "available_disk", json_object_new_int64(available));
        json_object_object_add(diskJson, "disk_usage_percent", json_object_new_double(usage_percent));

        return diskJson;
    }
    throw SystemInfoException("Failed to get disk stats");
}

/**
 * @brief Generates a JSON object containing complete system info.
 * 
 * The function collects memory, disk, hostname, and uptime information.
 * 
 * @return JSON object with system info.
 * @throws SystemInfoException if any part of the collection fails.
 */
static json_object *getSystemInfoJsonObj()
{
    json_object *jsonReport = json_object_new_object();
    if (!jsonReport)
        throw SystemInfoException("Failed to allocate JSON object for system info");

    json_object *memoryStats = nullptr;
    json_object *diskStats = nullptr;

    try
    {
        struct sysinfo info;
        int ret = sysinfo(&info);
        if (ret)
        {
            throw SystemInfoException("Failed to run sysinfo(). ret=" + to_string(errno));
        }

        memoryStats = getMemoryStatsJsonObj(&info);
        diskStats = getDiskStatsJsonObj();

        json_object_object_add(jsonReport, "memory", memoryStats);
        json_object_object_add(jsonReport, "disk", diskStats);
        json_object_object_add(jsonReport, "hostname", json_object_new_string(getHostname().c_str()));
        json_object_object_add(jsonReport, "uptime", json_object_new_int64(info.uptime));

        return jsonReport;
    }
    catch (...)
    {
        json_object_put(jsonReport);
        json_object_put(memoryStats);
        json_object_put(diskStats);
        throw;
    }
}

/**
 * @brief Constructs a SystemInfo object by collecting system data.
 * 
 * @throws SystemInfoException if data collection fails.
 */
SystemInfo::SystemInfo()
{
    jsonReport = getSystemInfoJsonObj();
}

/**
 * @brief Destructs the SystemInfo object, freeing allocated memory.
 */
SystemInfo::~SystemInfo()
{
    if (jsonReport)
        json_object_put(jsonReport);
}

/**
 * @brief Returns the system info as a formatted JSON string.
 * 
 * @return A JSON string with system info.
 */
std::string SystemInfo::getJsonStr() const
{
    const char *jsonStr = json_object_to_json_string_ext(jsonReport, JSON_C_TO_STRING_PRETTY);
    return std::string(jsonStr);
}

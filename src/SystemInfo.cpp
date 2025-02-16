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
#include <string>
#include <optional>
#include <expected>
#include "sys_utils.h"

using namespace ob;
using namespace std;

/**
 * @brief Retrieves the system hostname.
 *
 * @return expected<string, SystemInfo::sysstats_error> containing the hostname in case of success.
 */
static expected<string, SystemInfo::sysstats_error> getHostname()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
        return unexpected(SystemInfo::sysstats_error::failed_to_get_hostname);

    return string(hostname);
}

/**
 * @brief Retrieves memory statistics.
 *
 * Shows sizes in KiB just like `free`.
 *
 * @param info Pointer to a valid sysinfo structure.
 * @return expected<MemoryStats, SystemInfo::sysstats_error> containing memory statistics.
 */
static expected<MemoryStats, SystemInfo::sysstats_error> getMemoryStats(struct sysinfo *info)
{

    MemoryStats memory_info;
    memory_info.total = (info->totalram * info->mem_unit) / 1024;
    memory_info.free = (info->freeram * info->mem_unit) / 1024;
    memory_info.shared = info->sharedram / 1024;
    unsigned long cached_kb, available_kb, reclaimable_kb;
    unsigned long long cached_plus_free;

    int ret = parse_meminfo(&cached_kb, &available_kb, &reclaimable_kb);
    if (ret)
    {
        return unexpected(SystemInfo::sysstats_error::failed_to_parse_meminfo);
    }

    memory_info.available = ((unsigned long long)available_kb * 1024) / info->mem_unit;
    memory_info.cached = ((unsigned long long)cached_kb * 1024) / info->mem_unit;
    memory_info.cached += info->bufferram;
    memory_info.cached += ((unsigned long long)reclaimable_kb * 1024) / info->mem_unit;
    cached_plus_free = memory_info.cached + info->freeram;
    memory_info.used = (info->totalram - cached_plus_free) / 1024;

    return memory_info;
}

/**
 * @brief Retrieves disk statistics for the root filesystem.
 *
 * Shows sizes in KiB just like `df` and uses disk mounted at `/`.
 *
 * @return expected<DiskStats, SystemInfo::sysstats_error> containing disk statistics.
 */
static expected<DiskStats, SystemInfo::sysstats_error> getDiskStats()
{
    struct statfs s;
    if (statfs("/", &s) != 0)
    {
        return unexpected(SystemInfo::sysstats_error::failed_to_get_disk_stats);
    }

    DiskStats disk;

    long block_size = s.f_frsize;
    disk.total = s.f_blocks * block_size / 1024;
    disk.free = s.f_bfree * block_size / 1024;
    disk.available = s.f_bavail * block_size / 1024;
    disk.used = disk.total - disk.free;
    disk.usage_percentage = (disk.used * 100.0) / disk.total;

    return disk;
}

optional<SystemInfo::sysstats_error> SystemInfo::readSysInfo()
{
    struct sysinfo info;
    if (sysinfo(&info))
    {
        return sysstats_error::failed_to_get_sysinfo;
    }

    const auto hostname = getHostname();
    if (!hostname.has_value())
        return hostname.error();

    const auto memory = getMemoryStats(&info);
    if (!memory.has_value())
        return memory.error();

    const auto disk = getDiskStats();
    if (!disk.has_value())
        return disk.error();

    this->hostname = hostname.value();
    this->uptime = info.uptime;
    this->memory = memory.value();
    this->disk = disk.value();

    return {};
}

expected<const string, SystemInfo::json_error> SystemInfo::toJson()
{

    json_object *sysinfo_json_obj = json_object_new_object();
    if (!sysinfo_json_obj)
    {
        return unexpected(json_error::json_object_creation_error);
    }

    json_object *mem_json_obj = json_object_new_object();
    if (!mem_json_obj)
    {
        json_object_put(sysinfo_json_obj);
        return unexpected(json_error::json_object_creation_error);
    }

    json_object *disk_json_obj = json_object_new_object();
    if (!disk_json_obj)
    {
        json_object_put(sysinfo_json_obj);
        json_object_put(mem_json_obj);
        return unexpected(json_error::json_object_creation_error);
    }

    json_object_object_add(sysinfo_json_obj, "hostname", json_object_new_string(this->hostname.c_str()));
    json_object_object_add(sysinfo_json_obj, "uptime", json_object_new_int64(this->uptime));

    json_object_object_add(disk_json_obj, "total", json_object_new_int64(this->disk.total));
    json_object_object_add(disk_json_obj, "free", json_object_new_int64(this->disk.free));
    json_object_object_add(disk_json_obj, "used", json_object_new_int64(this->disk.used));
    json_object_object_add(disk_json_obj, "available", json_object_new_int64(this->disk.available));
    json_object_object_add(disk_json_obj, "usage_percentage", json_object_new_double(this->disk.usage_percentage));

    json_object_object_add(sysinfo_json_obj, "disk", disk_json_obj);

    json_object_object_add(mem_json_obj, "total", json_object_new_int64(this->memory.total));
    json_object_object_add(mem_json_obj, "used", json_object_new_int64(this->memory.used));
    json_object_object_add(mem_json_obj, "free", json_object_new_int64(this->memory.free));
    json_object_object_add(mem_json_obj, "shared", json_object_new_int64(this->memory.shared));
    json_object_object_add(mem_json_obj, "cached", json_object_new_int64(this->memory.cached));
    json_object_object_add(mem_json_obj, "available", json_object_new_int64(this->memory.available));

    json_object_object_add(sysinfo_json_obj, "memory", mem_json_obj);

    const char *json_str_c = json_object_to_json_string_ext(sysinfo_json_obj, JSON_C_TO_STRING_PRETTY);
    // copy to string so we can free the json objects
    string json_str = string(json_str_c);

    // Free only the parent object. The child objects will be freed automatically.
    json_object_put(sysinfo_json_obj);

    return json_str;
}

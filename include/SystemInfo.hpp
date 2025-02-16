/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#ifndef SYSTEMINFO_HPP
#define SYSTEMINFO_HPP

#include <string>
#include <stdexcept>
#include <expected>
#include <optional>
#include <json-c/json.h>

namespace ob
{

    /**
     * @struct DiskStats
     * @brief Holds disk usage statistics.
     *
     * The sizes are reported in KiB (kilobytes), similar to the output of the `df` command.
     */
    struct DiskStats
    {
        int64_t total;
        int64_t free;
        int64_t used;
        int64_t available;
        int64_t cached;
        int8_t usage_percentage;
    };

    /**
     * @struct MemoryStats
     * @brief Holds memory usage statistics.
     *
     * The memory sizes are reported in KiB (kilobytes), matching the output format of the `free` command.
     */
    struct MemoryStats
    {
        uint64_t total;
        uint64_t used;
        uint64_t free;
        uint64_t shared;
        uint64_t cached;
        uint64_t available;
    };

    /**
     * @class SystemInfo
     * @brief Collects and provides system information such as hostname, uptime, memory, and disk statistics.
     *
     * The SystemInfo class is responsible for retrieving various system statistics and providing
     * them both as raw data and as a JSON-formatted string. The class uses non-throwing methods 
     * that return error codes wrapped in std::optional or std::expected types.
     */
    class SystemInfo
    {
    public:
        /**
         * @enum json_error
         * @brief Error codes related to JSON serialization.
         */
        enum class json_error
        {
            json_object_creation_error ///< Failed to create a JSON object.
        };

        /**
         * @enum sysstats_error
         * @brief Error codes related to system statistics retrieval.
         */
        enum class sysstats_error
        {
            failed_to_get_hostname,    ///< Unable to retrieve the system hostname.
            failed_to_get_sysinfo,     ///< Unable to retrieve system uptime and memory info.
            failed_to_get_disk_stats,  ///< Unable to retrieve disk usage statistics.
            failed_to_parse_meminfo    ///< Unable to parse /proc/meminfo for detailed memory info.
        };

        /**
         * @brief Reads and populates the system information.
         *
         * This method collects system data including hostname, uptime, memory, and disk statistics.
         * All memory and disk sizes are reported in KiB.
         *
         * @return std::optional<sysstats_error>
         *         - An empty optional indicates success.
         *         - A non-empty optional contains the error code corresponding to the failure encountered.
         */
        std::optional<sysstats_error> readSysInfo();

        /**
         * @brief Serializes the system information to a JSON string.
         *
         * Converts the collected system statistics into a human-readable, pretty-printed JSON string.
         * On success, the function returns the JSON string wrapped in an std::expected.
         *
         * @return std::expected<const std::string, json_error>
         *         - On success, an expected containing the JSON string.
         *         - On failure (e.g., when a JSON object cannot be created), an unexpected containing the appropriate json_error.
         */
        std::expected<const std::string, json_error> toJson();

    private:
        std::string hostname; ///< System hostname.
        int64_t uptime;       ///< System uptime in seconds.
        DiskStats disk;       ///< Disk usage statistics.
        MemoryStats memory;   ///< Memory usage statistics.
    };
}

#endif // SYSTEMINFO_HPP

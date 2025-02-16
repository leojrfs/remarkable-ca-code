/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#ifndef SYSTEMINFO_HPP
#define SYSTEMINFO_HPP

#include <string>
#include <stdexcept>
#include <json-c/json.h>

/**
 * @brief Exception for system info errors.
 */
class SystemInfoException : public std::runtime_error {
public:
    /**
     * @brief Constructs a new SystemInfoException object.
     * @param message The error message.
     */
    explicit SystemInfoException(const std::string &message);
};

/**
 * @brief Collects system information and returns it as JSON.
 */
class SystemInfo {
public:
    /**
     * @brief Constructs a new SystemInfo object and collects system info.
     * @throws SystemInfoException if data collection fails.
     */
    SystemInfo();

    /**
     * @brief Destroys the SystemInfo object and frees allocated memory.
     */
    ~SystemInfo();

    /**
     * @brief Gets the system info as a formatted JSON string.
     * @return A string containing the JSON report.
     */
    std::string getJsonStr() const;

private:
    json_object *jsonReport; ///< JSON object holding the system info.
};

#endif // SYSTEMINFO_HPP

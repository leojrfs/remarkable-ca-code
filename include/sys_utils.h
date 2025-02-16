/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef SYS_UTILS_H
#define SYS_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif
unsigned int parse_meminfo(unsigned long *cached_kb, unsigned long *available_kb, unsigned long *reclaimable_kb);
#ifdef __cplusplus
}
#endif
#endif // SYS_UTILS_H
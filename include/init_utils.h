/*
 * Copyright (c) 2025 Leo Soares
 *
 * SPDX-License-Identifier: Proprietary
 */
#ifndef INIT_UITLS_H
#define INIT_UITLS_H
#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#define INIT_NOTIFY_READY() sd_notify(0, "READY=1")
#define INIT_NOTIFY_STOPPING() sd_notify(0, "STOPPING=1")
#define INIT_NOTIFY_WATCHDOG() sd_notify(0, "WATCHDOG=1")
#define INIT_NOTIFY_FAILED_TO_STARTUP(e) sd_notifyf(0, "STATUS=Failed to start up. ERRNO=%i", e)
#else
#define INIT_NOTIFY_READY()
#define INIT_NOTIFY_STOPPING()
#define INIT_NOTIFY_WATCHDOG()
#define INIT_NOTIFY_FAILED_TO_STARTUP(e)
#endif
#endif // INIT_UITLS_H

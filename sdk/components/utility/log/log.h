/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __FILE_NAME__
#define __FILE_NAME__                                                          \
    (strrchr(__FILE__, '/')                                                    \
         ? (strrchr(__FILE__, '/') + 1)                                        \
         : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))
#endif // __FILE_NAME__

#ifndef CONFIG_LOG_MAX_LEVEL
#define CONFIG_LOG_MAX_LEVEL LOG_LEVEL_INFO
#endif // CONFIG_LOG_MAX_LEVEL

#ifndef CONFIG_LOG_USE_COLOR
#define CONFIG_LOG_USE_COLOR (1)
#endif // CONFIG_LOG_USE_COLOR

#if CONFIG_LOG_USE_COLOR
#define LOG_PREFIX_INFO  "\x1b[32mINFO  \x1b[0m"
#define LOG_PREFIX_WARN  "\x1b[33mWARN  \x1b[0m"
#define LOG_PREFIX_ERROR "\x1b[31mERROR \x1b[0m"
#define LOG_PREFIX_USER  "\x1b[35mASSERT\x1b[0m"
#define LOG_PREFIX_DUMP  "\x1b[35mDUMP\x1b[0m"
#else
#define LOG_PREFIX_INFO  "INFO"
#define LOG_PREFIX_WARN  "WARN"
#define LOG_PREFIX_ERROR "ERROR"
#define LOG_PREFIX_USER  "ASSERT"
#define LOG_PREFIX_DUMP  "DUMP"
#endif

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    void* udata;
    int line;
    int level;
} log_Event;

typedef void (*log_LogFn)(log_Event* ev);

typedef enum _log_level {
    LOG_LEVEL_ALL = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_ASSERT,
    LOG_LEVEL_NEVER,
} log_level_t;

typedef struct {
    void* udata;
    int level;
} log_setting_t;

extern log_setting_t log_set;

#define custom_cflog(lowlevel, N, M, ...)                                      \
    do {                                                                       \
        if (lowlevel >= log_set.level) {                                       \
            log_printk("[%10u][%s: %s:%4d] " M "\r\n",                         \
                       (xPortIsInsideInterrupt())                              \
                           ? (xTaskGetTickCountFromISR())                      \
                           : (xTaskGetTickCount()),                            \
                       N, __FILE_NAME__, __LINE__, ##__VA_ARGS__);             \
        }                                                                      \
    } while (0 == 1)

#define custom_hexdumplog(name, lowlevel, logo, buf, size)                     \
    do {                                                                       \
        if (lowlevel >= log_set.level) {                                       \
            log_printk("[%10u][%s: %s:%4d] %s:\r\n",                           \
                       (xPortIsInsideInterrupt())                              \
                           ? (xTaskGetTickCountFromISR())                      \
                           : (xTaskGetTickCount()),                            \
                       logo, __FILE_NAME__, __LINE__, name);                   \
            log_hexdump_out(name, 16, buf, size);                              \
        }                                                                      \
    } while (0 == 1)

#define log_debug(M, ...)                                                      \
    custom_cflog(LOG_LEVEL_DEBUG, "DEBUG ", M, ##__VA_ARGS__);
#define log_info(M, ...)                                                       \
    custom_cflog(LOG_LEVEL_INFO, LOG_PREFIX_INFO, M, ##__VA_ARGS__);
#define log_warn(M, ...)                                                       \
    custom_cflog(LOG_LEVEL_WARN, LOG_PREFIX_WARN, M, ##__VA_ARGS__);
#define log_error(M, ...)                                                      \
    custom_cflog(LOG_LEVEL_ERROR, LOG_PREFIX_ERROR, M, ##__VA_ARGS__);

#define log_debug_hexdump(name, buf, size)                                     \
    custom_hexdumplog(name, LOG_LEVEL_DEBUG, "DEBUG ", buf, size);
#define log_info_hexdump(name, buf, size)                                      \
    custom_hexdumplog(name, LOG_LEVEL_INFO, LOG_PREFIX_INFO, buf, size);
#define log_warn_hexdump(name, buf, size)                                      \
    custom_hexdumplog(name, LOG_LEVEL_WARN, LOG_PREFIX_WARN, buf, size);
#define log_error_hexdump(name, buf, size)                                     \
    custom_hexdumplog(name, LOG_LEVEL_ERROR, LOG_PREFIX_ERROR, buf, size);

void log_set_level(int level);
void log_log(int level, const char* file, int line, const char* fmt, ...);
void log_dump(int level, const char* file, int line, const char* tag,
              const void* data, const size_t len);
void log_hexdump_out(const char* name, uint8_t width, const uint8_t* buf,
                     uint16_t size);

void log_printk(const char* format, ...);
#ifdef __cplusplus
}
#endif

#endif

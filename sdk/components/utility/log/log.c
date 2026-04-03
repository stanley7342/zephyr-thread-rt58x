/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

#ifndef CONFIG_LOG_NAMELEN_MAX
#define CONFIG_LOG_NAMELEN_MAX (128)
#endif // CONFIG_LOG_NAMELEN_MAX

#ifndef CONFIG_LOG_LINE_BUF_SIZE
#define CONFIG_LOG_LINE_BUF_SIZE (256)
#endif // CONFIG_LOG_LINE_BUF_SIZE

/* every line log's buffer */
static char log_buf[CONFIG_LOG_LINE_BUF_SIZE] = {0};

log_setting_t log_set = {NULL, CONFIG_LOG_MAX_LEVEL};

static const char* LS[] = {"DEBUG", "INFO", "WARN", "ERROR"};
static const char* LC[] = {"\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m"};

static size_t log_strcpy(size_t cur_len, char* dst, const char* src) {
    const char* src_old = src;

    if ((!dst) || (!src)) {
        printf("assert.\r\n");
        return 0;
    }

    while (*src != 0) {
        /* make sure destination has enough space */
        if (cur_len++ < CONFIG_LOG_LINE_BUF_SIZE)
            *dst++ = *src++;
        else
            break;
    }
    return src - src_old;
}

extern void vprint(const char* fmt, va_list argp);

void log_printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprint(format, args);
    va_end(args);
}

static void log_port_output(const char* log, size_t size) {
    /* output to terminal */
    log_printk("%.*s", size, log);
}

void log_hexdump_out(const char* name, uint8_t width, const uint8_t* buf,
                     uint16_t size) {
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

    uint16_t i, j;
    uint16_t log_len = 0;
    char dump_string[8] = {0};
    int fmt_result;
    for (i = 0; i < size; i += width) {
        /* package header */
        fmt_result = snprintf(log_buf, CONFIG_LOG_LINE_BUF_SIZE,
                              "[%s]: %04X-%04X: ", name, i, i + width - 1);
        /* calculate log length */
        log_len = ((fmt_result > -1)
                   && (fmt_result <= CONFIG_LOG_LINE_BUF_SIZE))
                      ? fmt_result
                      : CONFIG_LOG_LINE_BUF_SIZE;
        /* dump hex */
        for (j = 0; j < width; j++) {
            if (i + j < size) {
                snprintf(dump_string, sizeof(dump_string), "%02X ", buf[i + j]);
            } else {
                strncpy(dump_string, "   ", sizeof(dump_string));
            }
            log_len += log_strcpy(log_len, log_buf + log_len, dump_string);
            if ((j + 1) % 8 == 0) {
                log_len += log_strcpy(log_len, log_buf + log_len, " ");
            }
        }
        log_len += log_strcpy(log_len, log_buf + log_len, "  ");
        /* dump char for hex */
        for (j = 0; j < width; j++) {
            if (i + j < size) {
                snprintf(dump_string, sizeof(dump_string), "%c",
                         __is_print(buf[i + j]) ? buf[i + j] : '.');
                log_len += log_strcpy(log_len, log_buf + log_len, dump_string);
            }
        }
        /* overflow check and reserve some space for newline sign */
        if (log_len + strlen("\r\n") > CONFIG_LOG_LINE_BUF_SIZE)
            log_len = CONFIG_LOG_LINE_BUF_SIZE - strlen("\r\n");
        /* package newline sign */
        log_len += log_strcpy(log_len, log_buf + log_len, "\r\n");

        /* do log output */
        log_port_output(log_buf, log_len);
    }

    /* unlock output */
}

void log_set_level(int level) { log_set.level = level; }
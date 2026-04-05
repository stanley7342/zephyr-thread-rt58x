#pragma once

/*
 * tprintk.h — redefine printk to prepend millisecond uptime timestamp.
 *
 * Include AFTER <zephyr/kernel.h> (or any header that pulls it in).
 * k_uptime_get_32() must already be declared.
 *
 * Output format:  [  1234] [TAG] message
 *
 * To emit raw output (no timestamp) at a specific call site, write:
 *   (printk)("raw line\n");   <- parentheses bypass the macro
 */
#undef printk
#define printk(fmt, ...) \
    (printk)("[%7u] " fmt " [%s:%d]\n", k_uptime_get_32(), ##__VA_ARGS__, __FILE_NAME__, __LINE__)

#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/*
 * tprintk — printk with millisecond uptime timestamp
 *
 * Output format:  [  1234] [MAIN] message
 *                  ^^^^^^^
 *                  k_uptime_get32() — ms since boot (wraps ~49 days)
 *
 * RULE: same as printk — never call from ISR context.
 */
#define tprintk(fmt, ...) \
    printk("[%7u] " fmt, k_uptime_get_32(), ##__VA_ARGS__)

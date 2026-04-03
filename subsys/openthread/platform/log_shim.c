/*
 * log_shim.c — Rafael SDK log compatibility for Zephyr
 *
 * liblog.a references `vprint` (a Rafael-internal print helper).
 * We provide a thin wrapper that forwards to Zephyr's vprintk.
 */

#include <stdarg.h>
#include <zephyr/sys/printk.h>

void vprint(const char *fmt, va_list args)
{
    vprintk(fmt, args);
}

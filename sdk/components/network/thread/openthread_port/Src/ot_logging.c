/*
 * ot_logging.c — OpenThread logging platform for Zephyr
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <openthread/config.h>
#include <openthread/platform/logging.h>
#include "tprintk.h"

void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion,
               const char *aFormat, ...)
{
    (void)aLogLevel;
    (void)aLogRegion;

    char buf[256];
    va_list args;
    va_start(args, aFormat);
    vsnprintf(buf, sizeof(buf), aFormat, args);
    va_end(args);
    printk("%s", buf);
}

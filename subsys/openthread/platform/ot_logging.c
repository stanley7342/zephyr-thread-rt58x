/*
 * ot_logging.c — OpenThread logging platform for Zephyr
 * vprint() replaced with vprintk().
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <zephyr/kernel.h>

#include <openthread/config.h>
#include <openthread/platform/logging.h>

void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion,
               const char *aFormat, ...)
{
    (void)aLogLevel;
    (void)aLogRegion;

    va_list args;
    char buf[256];
    va_start(args, aFormat);
    vsnprintf(buf, sizeof(buf), aFormat, args);
    va_end(args);
    printk("%s\r\n", buf);
}

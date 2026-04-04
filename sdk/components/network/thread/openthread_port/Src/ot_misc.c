/*
 * ot_misc.c — OpenThread misc platform for Zephyr on RT582
 * log_info/error replaced with printk; WDT APIs kept from Rafael SDK.
 */

#include <zephyr/kernel.h>
#include <openthread/platform/misc.h>
#include <openthread/platform/radio.h>
#include <string.h>
#include <stdio.h>

#include "mcu.h"
#include "wdt.h"
#include "tprintk.h"

#define ms_sec(N) ((N) * 1000)

void otPlatReset(otInstance *aInstance)
{
    (void)aInstance;
    printk("[OT] otPlatReset\n");

    wdt_config_mode_t wdt_mode = {
        .int_enable    = 0,
        .reset_enable  = 1,
        .lock_enable   = 1,
        .prescale      = WDT_PRESCALE_32,
    };
    wdt_config_tick_t wdt_cfg = {
        .wdt_ticks     = ms_sec(1200),
        .int_ticks     = ms_sec(0),
        .wdt_min_ticks = ms_sec(1),
    };

    /* Set retention regs to signal reboot reason */
    sys_set_retention_reg(6, 7);
    sys_set_retention_reg(7, 0);

    wdt_start(wdt_mode, wdt_cfg, NULL);
    while (1) { /* wait for WDT reset */ }
}

otPlatResetReason otPlatGetResetReason(otInstance *aInstance)
{
    (void)aInstance;
    return OT_PLAT_RESET_REASON_POWER_ON;
}

void otPlatWakeHost(void) {}

void otPlatAssertFail(const char *aFilename, int aLineNumber)
{
    printk("[OT] Assert failed: %s @ %d\n", aFilename, aLineNumber);
}

/* ── CSL stubs ───────────────────────────────────────────────────────────── *
 * RT582 hardware does not implement Coordinated Sampled Listening (CSL).   */
otError otPlatRadioEnableCsl(otInstance *aInstance, uint32_t aCslPeriod,
                              otShortAddress aShortAddr,
                              const otExtAddress *aExtAddr)
{
    (void)aInstance; (void)aCslPeriod; (void)aShortAddr; (void)aExtAddr;
    return OT_ERROR_NOT_IMPLEMENTED;
}

void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime)
{
    (void)aInstance; (void)aCslSampleTime;
}

/* ── Enhanced ACK probing stub ───────────────────────────────────────────── */
otError otPlatRadioConfigureEnhAckProbing(otInstance          *aInstance,
                                          otLinkMetrics        aLinkMetrics,
                                          otShortAddress       aShortAddress,
                                          const otExtAddress  *aExtAddress)
{
    (void)aInstance; (void)aLinkMetrics; (void)aShortAddress; (void)aExtAddress;
    return OT_ERROR_NOT_IMPLEMENTED;
}

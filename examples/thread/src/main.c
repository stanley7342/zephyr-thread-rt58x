/*
 * main.c — OpenThread FTD CLI example (rt583_evb / rt584_evb).
 *
 * Build:
 *   west build --sysbuild -p always -b rt583_evb examples/thread -d build/thread583
 *   west build --sysbuild -p always -b rt584_evb examples/thread -d build/thread584
 *
 * Flash:
 *   west flash -d build/thread583   # or build/thread584
 *
 * RULE: Never call printk from a k_timer expiry or any ISR.
 * k_timer expiry runs in SysTick ISR context. printk holds a spinlock;
 * on a single-core Cortex-M3 this causes deadlock if main thread is
 * also inside printk when the ISR fires. Use k_work_submit instead.
 */

#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/crc.h>
#include "tprintk.h"

#include <openthread/cli.h>
#include <openthread/instance.h>

#include "openthread_port.h"
#if defined(CONFIG_OTA)
#include "ota_app.h"
#endif
#include "hosal_rf.h"
#include "lmac15p4.h"

/* RF firmware blob — integrity verified before loading */
extern const uint8_t  firmware_program_ruci[];
extern const uint32_t firmware_size_ruci;

/* ── Watchdog thread (independent of system workqueue) ───────────────────── */
#define WDOG_STACK_SIZE 1024
K_THREAD_STACK_DEFINE(wdog_stack, WDOG_STACK_SIZE);
static struct k_thread wdog_thread_data;

static void wdog_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    while (1) {
        k_msleep(1000);
    }
}

/* PHY/MAC PIB — tunable via Kconfig (prj.conf CONFIG_RT583_*_PIB_*) */
#define PHY_PIB_TURNAROUND_TIMER          CONFIG_RT583_PHY_PIB_TURNAROUND_TIMER
#define PHY_PIB_CCA_DETECTED_TIME         CONFIG_RT583_PHY_PIB_CCA_DETECTED_TIME
#define PHY_PIB_CCA_DETECT_MODE           CONFIG_RT583_PHY_PIB_CCA_DETECT_MODE
#define PHY_PIB_CCA_THRESHOLD             CONFIG_RT583_PHY_PIB_CCA_THRESHOLD
#define MAC_PIB_UNIT_BACKOFF_PERIOD       CONFIG_RT583_MAC_PIB_UNIT_BACKOFF_PERIOD
#define MAC_PIB_MAC_ACK_WAIT_DURATION     CONFIG_RT583_MAC_PIB_ACK_WAIT_DURATION
#define MAC_PIB_MAC_MAX_BE                CONFIG_RT583_MAC_PIB_MAX_BE
#define MAC_PIB_MAC_MAX_FRAME_TOTAL_WAIT_TIME CONFIG_RT583_MAC_PIB_MAX_FRAME_TOTAL_WAIT_TIME
#define MAC_PIB_MAC_MAX_FRAME_RETRIES     CONFIG_RT583_MAC_PIB_MAX_FRAME_RETRIES
#define MAC_PIB_MAC_MAX_CSMACA_BACKOFFS   CONFIG_RT583_MAC_PIB_MAX_CSMACA_BACKOFFS
#define MAC_PIB_MAC_MIN_BE                CONFIG_RT583_MAC_PIB_MIN_BE

void otrInitUser(otInstance *instance)
{
    otAppCliInit(instance);
#if defined(CONFIG_OTA)
    ota_app_init(instance);
#endif
}

int main(void)
{
    (printk)("======================================\n");
    (printk)("  %s  Zephyr + OpenThread CLI\n", CONFIG_BOARD);
    (printk)("  Built: " __DATE__ " " __TIME__ "\n");
    (printk)("======================================\n");

    k_thread_create(&wdog_thread_data, wdog_stack, K_THREAD_STACK_SIZEOF(wdog_stack),
                    wdog_thread_fn, NULL, NULL, NULL,
                    2, 0, K_NO_WAIT);
    k_thread_name_set(&wdog_thread_data, "wdog");

    /* Verify RF firmware blob integrity before loading */
    if (firmware_size_ruci > 0) {
        uint32_t fw_crc = crc32_ieee(firmware_program_ruci, firmware_size_ruci);
        printk("[RF] firmware CRC32: 0x%08x (size=%u)", fw_crc, firmware_size_ruci);
#if CONFIG_RT583_RF_FW_EXPECTED_CRC32 != 0
        if (fw_crc != CONFIG_RT583_RF_FW_EXPECTED_CRC32) {
            printk("[RF] ERROR: firmware CRC mismatch! expected=0x%08x",
                   CONFIG_RT583_RF_FW_EXPECTED_CRC32);
        }
#endif
    }

    printk("[RF] hosal_rf_init...");
    hosal_rf_init(HOSAL_RF_MODE_RUCI_CMD);
    irq_enable(20); /* COMM_SUBSYSTEM IRQ — must enable after hosal_rf_init sets isr callback */
    printk("[RF] hosal_rf_init done");

    printk("[RF] lmac15p4_init...");
    lmac15p4_init(LMAC15P4_2P4G_OQPSK, 0);
    printk("[RF] lmac15p4_init done");

    lmac15p4_phy_pib_set(PHY_PIB_TURNAROUND_TIMER, PHY_PIB_CCA_DETECT_MODE,
                         PHY_PIB_CCA_THRESHOLD, PHY_PIB_CCA_DETECTED_TIME);
    lmac15p4_mac_pib_set(MAC_PIB_UNIT_BACKOFF_PERIOD,
                         MAC_PIB_MAC_ACK_WAIT_DURATION, MAC_PIB_MAC_MAX_BE,
                         MAC_PIB_MAC_MAX_CSMACA_BACKOFFS,
                         MAC_PIB_MAC_MAX_FRAME_TOTAL_WAIT_TIME,
                         MAC_PIB_MAC_MAX_FRAME_RETRIES, MAC_PIB_MAC_MIN_BE);

    otrStart();
    printk("OpenThread FTD CLI started.");

    return 0;
}

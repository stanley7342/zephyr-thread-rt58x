/*
 * main.c — RT582-EVB Zephyr + OpenThread CLI
 *
 * RULE: Never call printk from a k_timer expiry or any ISR.
 * k_timer expiry runs in SysTick ISR context. printk holds a spinlock;
 * if the main thread is also inside printk the ISR will spin forever
 * (single-core deadlock). Use k_work_submit to defer to thread context.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <openthread/cli.h>
#include <openthread/instance.h>

#include "openthread_port.h"
#include "hosal_rf.h"
#include "lmac15p4.h"

extern volatile uint32_t rt582_comm_irq_count;
extern volatile uint32_t ot_uart_rx_byte_count;
extern volatile uint32_t ot_uart_isr_count;
extern volatile uint32_t ot_uart_line_count;

/* ── Watchdog: dedicated thread (independent of system workqueue) ────────── *
 * k_msleep-based — does not rely on k_timer/k_work/system workqueue.        *
 * If this prints but k_work version does not → workqueue stack overflow.    *
 * If this does not print → scheduler not running (IRQs blocked).            */
#define WDOG_STACK_SIZE 1024
K_THREAD_STACK_DEFINE(wdog_stack, WDOG_STACK_SIZE);
static struct k_thread wdog_thread_data;
static uint32_t        wdog_tick;

static void wdog_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    while (1) {
        k_msleep(200);
        wdog_tick++;
    }
}

/* PIB values — configured via Kconfig (RT582 IEEE 802.15.4 PHY/MAC PIB menu) */
#define PHY_PIB_TURNAROUND_TIMER          CONFIG_RT582_PHY_PIB_TURNAROUND_TIMER
#define PHY_PIB_CCA_DETECTED_TIME         CONFIG_RT582_PHY_PIB_CCA_DETECTED_TIME
#define PHY_PIB_CCA_DETECT_MODE           CONFIG_RT582_PHY_PIB_CCA_DETECT_MODE
#define PHY_PIB_CCA_THRESHOLD             CONFIG_RT582_PHY_PIB_CCA_THRESHOLD
#define MAC_PIB_UNIT_BACKOFF_PERIOD       CONFIG_RT582_MAC_PIB_UNIT_BACKOFF_PERIOD
#define MAC_PIB_MAC_ACK_WAIT_DURATION     CONFIG_RT582_MAC_PIB_ACK_WAIT_DURATION
#define MAC_PIB_MAC_MAX_BE                CONFIG_RT582_MAC_PIB_MAX_BE
#define MAC_PIB_MAC_MAX_FRAME_TOTAL_WAIT_TIME CONFIG_RT582_MAC_PIB_MAX_FRAME_TOTAL_WAIT_TIME
#define MAC_PIB_MAC_MAX_FRAME_RETRIES     CONFIG_RT582_MAC_PIB_MAX_FRAME_RETRIES
#define MAC_PIB_MAC_MAX_CSMACA_BACKOFFS   CONFIG_RT582_MAC_PIB_MAX_CSMACA_BACKOFFS
#define MAC_PIB_MAC_MIN_BE                CONFIG_RT582_MAC_PIB_MIN_BE

void otrInitUser(otInstance *instance)
{
    otAppCliInit(instance);
}

int main(void)
{
    printk("======================================\n");
    printk("  RT582-EVB  Zephyr + OpenThread CLI  \n");
    printk("======================================\n");

    k_thread_create(&wdog_thread_data, wdog_stack, K_THREAD_STACK_SIZEOF(wdog_stack),
                    wdog_thread_fn, NULL, NULL, NULL,
                    2, 0, K_NO_WAIT);
    k_thread_name_set(&wdog_thread_data, "wdog");

    printk("[MAIN] wdog started\n");

    printk("[RF] hosal_rf_init...\n");
    hosal_rf_init(HOSAL_RF_MODE_RUCI_CMD);
    printk("[RF] hosal_rf_init done\n");

    printk("[RF] lmac15p4_init...\n");
    lmac15p4_init(LMAC15P4_2P4G_OQPSK, 0);
    printk("[RF] lmac15p4_init done\n");

    lmac15p4_phy_pib_set(PHY_PIB_TURNAROUND_TIMER, PHY_PIB_CCA_DETECT_MODE,
                         PHY_PIB_CCA_THRESHOLD, PHY_PIB_CCA_DETECTED_TIME);
    lmac15p4_mac_pib_set(MAC_PIB_UNIT_BACKOFF_PERIOD,
                         MAC_PIB_MAC_ACK_WAIT_DURATION, MAC_PIB_MAC_MAX_BE,
                         MAC_PIB_MAC_MAX_CSMACA_BACKOFFS,
                         MAC_PIB_MAC_MAX_FRAME_TOTAL_WAIT_TIME,
                         MAC_PIB_MAC_MAX_FRAME_RETRIES, MAC_PIB_MAC_MIN_BE);
    printk("[RF] PIB set done\n");

    otrStart();
    printk("OpenThread FTD task started.\n");

    return 0;
}

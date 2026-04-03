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

/* ── Watchdog: ISR sets tick, workqueue thread prints ────────────────────── */
static struct k_work wdog_work;
static uint32_t      wdog_tick;

static void wdog_work_handler(struct k_work *w)
{
    volatile uint32_t *tx_info   = (volatile uint32_t *)(0xA0400000UL + 0x20UL);
    volatile uint32_t *rx_info   = (volatile uint32_t *)(0xA0400000UL + 0x1CUL);
    volatile uint32_t *comm_info = (volatile uint32_t *)(0xA0400000UL + 0x24UL);
    volatile uint32_t *intr_en   = (volatile uint32_t *)(0xA0400000UL + 0x10UL);
    volatile uint32_t *intr_stat = (volatile uint32_t *)(0xA0400000UL + 0x18UL);
    volatile uint32_t *nvic_ispr = (volatile uint32_t *)(0xE000E200UL);
    volatile uint32_t *nvic_iabr = (volatile uint32_t *)(0xE000E300UL);
    volatile uint8_t  *nvic_ipr20= (volatile uint8_t  *)(0xE000E414UL);
    uint32_t mcu = (*tx_info & 0xFF00U) >> 8;
    uint32_t bp;
    __asm__ volatile ("mrs %0, basepri" : "=r"(bp));

    printk("[WDG] t=%us IRQ=%u UART_RX=%u TX=0x%08x RX=0x%08x IE=0x%08x INFO=0x%08x MCU=0x%02x\n",
           wdog_tick, rt582_comm_irq_count, ot_uart_rx_byte_count,
           *tx_info, *rx_info, *intr_en, *comm_info, mcu);
    printk("[WDG] INTR_STAT=0x%08x ISPR=0x%08x IABR=0x%08x IPR20=0x%02x BASEPRI=0x%02x\n",
           *intr_stat, *nvic_ispr, *nvic_iabr, (uint32_t)*nvic_ipr20, bp);
}

static void wdog_expiry(struct k_timer *t)
{
    wdog_tick++;
    k_work_submit(&wdog_work);  /* defer printk to workqueue thread */
}
K_TIMER_DEFINE(wdog_timer, wdog_expiry, NULL);

/* PIB values */
#define PHY_PIB_TURNAROUND_TIMER          192
#define PHY_PIB_CCA_DETECTED_TIME         128
#define PHY_PIB_CCA_DETECT_MODE             0
#define PHY_PIB_CCA_THRESHOLD              85
#define MAC_PIB_UNIT_BACKOFF_PERIOD       320
#define MAC_PIB_MAC_ACK_WAIT_DURATION     544
#define MAC_PIB_MAC_MAX_BE                  8
#define MAC_PIB_MAC_MAX_FRAME_TOTAL_WAIT_TIME 16416
#define MAC_PIB_MAC_MAX_FRAME_RETRIES       4
#define MAC_PIB_MAC_MAX_CSMACA_BACKOFFS    10
#define MAC_PIB_MAC_MIN_BE                  5

void otrInitUser(otInstance *instance)
{
    otAppCliInit(instance);
}

int main(void)
{
    printk("======================================\n");
    printk("  RT582-EVB  Zephyr + OpenThread CLI  \n");
    printk("======================================\n");

    k_work_init(&wdog_work, wdog_work_handler);
    k_timer_start(&wdog_timer, K_SECONDS(1), K_SECONDS(1));

    {
        volatile uint32_t *comm_info = (volatile uint32_t *)(0xA0400000UL + 0x24UL);
        volatile uint32_t *comm_host = (volatile uint32_t *)(0xA0400000UL + 0x0CUL);
        volatile uint32_t *comm_base = (volatile uint32_t *)(0xA0400000UL + 0x00UL);
        printk("[RF] COMM_BASE=0x%08x COMM_HOST=0x%08x COMM_INFO=0x%08x\n",
               *comm_base, *comm_host, *comm_info);
    }

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

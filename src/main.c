/*
 * main.c — RT582-EVB Zephyr + OpenThread CLI demo
 *
 * Starts the OpenThread FTD stack and exposes an 'ot' shell command
 * so you can type OT CLI commands over UART0 (115200 8N1).
 *
 * Example session (via Tera Term / PuTTY):
 *   ot state         → disabled
 *   ot dataset init new
 *   ot dataset commit active
 *   ot ifconfig up
 *   ot thread start
 *   ot state         → leader
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>

#include <openthread/cli.h>
#include <openthread/instance.h>

#include "openthread_port.h"
#include "hosal_rf.h"
#include "lmac15p4.h"

/* IRQ fire counter defined in soc.c */
extern volatile uint32_t rt582_comm_irq_count;

/* ── ISR-context watchdog ────────────────────────────────────────────────── *
 * k_timer expiry runs from the SysTick ISR, so it fires even when main()   *
 * is stuck in a tight spin-loop and never yields to the scheduler.          *
 * Prints COMM_SUBSYSTEM_AHB state every second to track what changed.      */
static void wdog_expiry(struct k_timer *t)
{
    static uint32_t tick;
    /* COMM_SUBSYSTEM_AHB register map (base 0xA0400000):
     *   0x0C HOST,  0x10 IE,     0x14 INTR_CLR,  0x18 INTR_STAT,
     *   0x1C RX_INFO, 0x20 TX_INFO, 0x24 INFO
     * TX_INFO[15:8] = MCU_STATE:
     *   0x01 = RF_MCU_STATE_INIT_SUCCEED (what SysInitNotify waits for)
     *   0x02 = RF_MCU_STATE_EVENT_DONE
     * INFO[0]=SYS_RDY, INFO[2:1]=PWR_STATE (3=NORMAL)
     *
     * NVIC (Cortex-M, base 0xE000E000):
     *   ISPR[0] 0xE000E200 — pending IRQs  (bit 20 = CommSubsystem pending)
     *   IABR[0] 0xE000E300 — active IRQs   (bit 20 = CommSubsystem active)
     *   IPR[5]  0xE000E414 — priority byte for IRQ 20 (bits[7:5] count)
     *   BASEPRI 0xE000ED20 — current BASEPRI mask
     */
    volatile uint32_t *tx_info      = (volatile uint32_t *)(0xA0400000UL + 0x20UL);
    volatile uint32_t *rx_info      = (volatile uint32_t *)(0xA0400000UL + 0x1CUL);
    volatile uint32_t *comm_info    = (volatile uint32_t *)(0xA0400000UL + 0x24UL);
    volatile uint32_t *intr_en      = (volatile uint32_t *)(0xA0400000UL + 0x10UL);
    volatile uint32_t *intr_stat    = (volatile uint32_t *)(0xA0400000UL + 0x18UL);
    volatile uint32_t *nvic_ispr    = (volatile uint32_t *)(0xE000E200UL);  /* ISPR[0] */
    volatile uint32_t *nvic_iabr    = (volatile uint32_t *)(0xE000E300UL);  /* IABR[0] */
    volatile uint8_t  *nvic_ipr20   = (volatile uint8_t  *)(0xE000E414UL);  /* IPR[20] */

    uint32_t irq  = rt582_comm_irq_count;
    /* MCU_STATE is TX_INFO bits[15:8], not bits[7:0] (those are TX_Q_W_RDY) */
    uint32_t mcu  = (*tx_info & 0xFF00U) >> 8;
    uint32_t istat = *intr_stat;
    uint32_t ispr  = *nvic_ispr;
    uint32_t iabr  = *nvic_iabr;
    uint32_t ipr20 = (uint32_t)*nvic_ipr20;
    /* BASEPRI is a special register; read via inline asm on Cortex-M */
    uint32_t bp;
    __asm__ volatile ("mrs %0, basepri" : "=r"(bp));

    printk("[WDG] t=%us IRQ=%u TX=0x%08x RX=0x%08x IE=0x%08x INFO=0x%08x MCU=0x%02x\n",
           ++tick, irq, *tx_info, *rx_info, *intr_en, *comm_info, mcu);
    printk("[WDG] INTR_STAT=0x%08x ISPR=0x%08x IABR=0x%08x IPR20=0x%02x BASEPRI=0x%02x\n",
           istat, ispr, iabr, ipr20, bp);
}
K_TIMER_DEFINE(wdog_timer, wdog_expiry, NULL);

/* PIB values from SDK ot-cli-ftd example */
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

/* ── otrInitUser ─────────────────────────────────────────────────────────── *
 * Called once by otrStackTask after the OT instance is ready.              */
void otrInitUser(otInstance *instance)
{
    otAppCliInit(instance);
}

/* ── Zephyr shell 'ot' command ───────────────────────────────────────────── *
 * Forwards shell input to the OT task via ot_uartRecieved().               */
static int cmd_ot(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_print(sh, "Usage: ot <openthread-command> [args...]");
        return 0;
    }

    char buf[256] = {0};
    for (int i = 1; i < (int)argc; i++) {
        if (i > 1) {
            strncat(buf, " ", sizeof(buf) - strlen(buf) - 1);
        }
        strncat(buf, argv[i], sizeof(buf) - strlen(buf) - 1);
    }

    printk("[SHELL] cmd_ot: '%s'\n", buf);
    ot_uartRecieved((uint8_t *)buf, (uint32_t)strlen(buf));
    return 0;
}

SHELL_CMD_ARG_REGISTER(ot, NULL, "OpenThread CLI passthrough", cmd_ot, 1, 16);

/* ── main ────────────────────────────────────────────────────────────────── */
int main(void)
{
    printk("======================================\n");
    printk("  RT582-EVB  Zephyr + OpenThread CLI  \n");
    printk("======================================\n");

    /* Start ISR-context watchdog — fires every second via SysTick, even
     * while main() is stuck in a tight spin-loop. */
    k_timer_start(&wdog_timer, K_SECONDS(1), K_SECONDS(1));

    /* RF and LMAC must be initialised before the OT thread calls
     * lmac15p4_* functions, matching the SDK's app_main_entry sequence. */
    /* Debug: read COMM_SUBSYSTEM_AHB registers before RF init */
    {
        volatile uint32_t *comm_info = (volatile uint32_t *)(0xA0400000UL + 0x24UL);
        volatile uint32_t *comm_host = (volatile uint32_t *)(0xA0400000UL + 0x0CUL);
        volatile uint32_t *comm_base = (volatile uint32_t *)(0xA0400000UL + 0x00UL);
        printk("[RF] COMM_BASE=0x%08x COMM_HOST=0x%08x COMM_INFO=0x%08x\n",
               *comm_base, *comm_host, *comm_info);
        printk("[RF] SYS_RDY bit=%d\n", (int)(*comm_info & 0x1U));
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
    printk("Type 'ot <cmd>' in the shell, e.g.: ot state\n");

    return 0;
}

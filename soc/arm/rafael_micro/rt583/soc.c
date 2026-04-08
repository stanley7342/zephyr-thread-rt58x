/*
 * RT583 SoC initialisation for Zephyr.
 *
 * Must run at PRE_KERNEL_1 priority 0 so the COMM_SUBSYSTEM_AHB is ready
 * before any driver (UART, RF) accesses it.
 *
 * RULE: Never call printk here. UART driver runs at PRE_KERNEL_1 priority 50
 * (after this function). hosal_uart_send polls LSR.THRE which spins forever
 * if the UART clock has not been enabled yet.
 */

#include <zephyr/init.h>
#include <zephyr/irq.h>

extern void SystemInit(void);

#define RT583_COMM_SUBSYSTEM_IRQN   20
#define RT583_COMM_SUBSYSTEM_IRQPRI 2

#if defined(CONFIG_OPENTHREAD_RT583) || defined(CONFIG_BLE_RT583)
extern void RfMcu_IsrHandler(void);

volatile uint32_t rt583_comm_irq_count;

static void comm_subsystem_isr(const void *arg)
{
    ARG_UNUSED(arg);
    rt583_comm_irq_count++;
    RfMcu_IsrHandler();
}
#endif /* CONFIG_OPENTHREAD_RT583 || CONFIG_BLE_RT583 */

static int rt583_soc_init(void)
{
    SystemInit(); /* Required: configures BBPLL to 64 MHz (from librt583_system.a).
                   * Without this the UART peripheral has no clock and
                   * hosal_uart_send_complete spins on LSR.TEMT forever. */

#if defined(CONFIG_OPENTHREAD_RT583) || defined(CONFIG_BLE_RT583)
    IRQ_CONNECT(RT583_COMM_SUBSYSTEM_IRQN,
                RT583_COMM_SUBSYSTEM_IRQPRI,
                comm_subsystem_isr,
                NULL, 0);
    /* Do NOT irq_enable here. gRfMcuIsrCfg.commsubsystem_isr is still NULL
     * until hosal_rf_init() is called by the subsystem driver. Enabling the
     * IRQ here causes a NULL function-pointer crash on warm reset when the
     * RF MCU has a pending interrupt (e.g. from the previous boot session).
     * Each subsystem enables IRQ_20 itself after hosal_rf_init(). */
#endif /* CONFIG_OPENTHREAD_RT583 || CONFIG_BLE_RT583 */

    return 0;
}

SYS_INIT(rt583_soc_init, PRE_KERNEL_1, 0);

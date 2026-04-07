/*
 * RT582 SoC initialisation for Zephyr.
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

#define RT582_COMM_SUBSYSTEM_IRQN   20
#define RT582_COMM_SUBSYSTEM_IRQPRI 2

#if defined(CONFIG_OPENTHREAD_RT582) || defined(CONFIG_BLE_RT582)
extern void RfMcu_IsrHandler(void);

volatile uint32_t rt582_comm_irq_count;

static void comm_subsystem_isr(const void *arg)
{
    ARG_UNUSED(arg);
    rt582_comm_irq_count++;
    RfMcu_IsrHandler();
}
#endif /* CONFIG_OPENTHREAD_RT582 || CONFIG_BLE_RT582 */

static int rt582_soc_init(void)
{
    SystemInit(); /* Required: configures BBPLL to 64 MHz (from librt582_system.a).
                   * Without this the UART peripheral has no clock and
                   * hosal_uart_send_complete spins on LSR.TEMT forever. */

#if defined(CONFIG_OPENTHREAD_RT582) || defined(CONFIG_BLE_RT582)
    IRQ_CONNECT(RT582_COMM_SUBSYSTEM_IRQN,
                RT582_COMM_SUBSYSTEM_IRQPRI,
                comm_subsystem_isr,
                NULL, 0);
    irq_enable(RT582_COMM_SUBSYSTEM_IRQN);
#endif /* CONFIG_OPENTHREAD_RT582 || CONFIG_BLE_RT582 */

    return 0;
}

SYS_INIT(rt582_soc_init, PRE_KERNEL_1, 0);

/*
 * RT582 SoC initialisation for Zephyr.
 *
 * Calls the Rafael SDK SystemInit() which configures:
 *   - PMU (XTAL, LDO/DCDC settings)
 *   - Flash QSPI mode
 *   - MP sector (trim/calibration data)
 *   - AHB system clock (48 MHz default)
 *   - RCO 40 kHz analog calibration
 *
 * Also wires CommSubsystem_IRQn (IRQ #20) → RfMcu_IsrHandler() so that
 * rf_common_init_by_fw() can complete after the RF co-processor sets SYS_RDY.
 *
 * Must run at PRE_KERNEL_1 priority 0 so the COMM_SUBSYSTEM_AHB is ready
 * before any driver (UART, RF) accesses it.
 */

#include <zephyr/init.h>
#include <zephyr/irq.h>
#include <zephyr/sys/printk.h>

/* Declared in librt582_system.a (system_mcu.c) */
extern void SystemInit(void);

/* Declared in librt569-rf.a (rf_mcu.c) */
extern void RfMcu_IsrHandler(void);

/* CommSubsystem_IRQn = 20 on RT582 (CMSIS peripheral IRQ number) */
#define RT582_COMM_SUBSYSTEM_IRQN   20
#define RT582_COMM_SUBSYSTEM_IRQPRI 2

/* Count how many times the CommSubsystem IRQ fires — readable from main.c */
volatile uint32_t rt582_comm_irq_count;

/* Wrapper — keep lean: no printk inside, DMA-done IRQs are time-critical */
static void comm_subsystem_isr(const void *arg)
{
    ARG_UNUSED(arg);
    rt582_comm_irq_count++;
    RfMcu_IsrHandler();
}

static int rt582_soc_init(void)
{
    SystemInit();

    /* Connect and enable the RF co-processor interrupt.
     * rf_common_init_by_fw() calls __NVIC_EnableIRQ(CommSubsystem_IRQn)
     * internally, but IRQ_CONNECT must happen at compile-time before that. */
    IRQ_CONNECT(RT582_COMM_SUBSYSTEM_IRQN,
                RT582_COMM_SUBSYSTEM_IRQPRI,
                comm_subsystem_isr,
                NULL, 0);
    irq_enable(RT582_COMM_SUBSYSTEM_IRQN);

    printk("[SOC] SystemInit done, CommSubsystem IRQ %d enabled\n",
           RT582_COMM_SUBSYSTEM_IRQN);
    return 0;
}

SYS_INIT(rt582_soc_init, PRE_KERNEL_1, 0);

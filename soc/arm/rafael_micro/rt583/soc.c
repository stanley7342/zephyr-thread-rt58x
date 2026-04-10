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

/* ── Bare-metal UART0 early diagnostic ──────────────────────────────────── *
 * Writes directly to UART0 hardware registers with proper pin mux, clock,   *
 * and baud setup so we see output at 115200 immediately after SystemInit.   *
 *                                                                            *
 * SYSCTRL register map (base 0x40800000):                                   *
 *   +0x04  sys_clk_ctrl  — bit 16 = UART0_CLK enable                       *
 *   +0x10  pin_map[0]    — GPIO0..7   (4 bits each, offset by pin&7*4)      *
 *   +0x14  pin_map[1]    — GPIO8..15                                        *
 *   +0x18  pin_map[2]    — GPIO16..23  MODE_UART=6                          *
 *                          GPIO16 bits [3:0], GPIO17 bits [7:4]             *
 *                                                                            *
 * UART0 register map (base 0xA0000000, 4-byte spacing):                    *
 *   +0x000 THR/RBR/DLL   +0x004 IER/DLM   +0x00C LCR   +0x014 LSR         */
#define RT583_UART0_BASE        0xA0000000UL
#define RT583_SYSCTRL_CLKCTRL  (*(volatile uint32_t *)0x40800004UL)
#define RT583_PINMAP2          (*(volatile uint32_t *)0x40800018UL)  /* GPIO16..23 */
#define UART0_THR  (*(volatile uint32_t *)(RT583_UART0_BASE + 0x000))
#define UART0_DLL  (*(volatile uint32_t *)(RT583_UART0_BASE + 0x000)) /* when DLAB=1 */
#define UART0_DLM  (*(volatile uint32_t *)(RT583_UART0_BASE + 0x004)) /* when DLAB=1 */
#define UART0_LCR  (*(volatile uint32_t *)(RT583_UART0_BASE + 0x00C))
#define UART0_LSR  (*(volatile uint32_t *)(RT583_UART0_BASE + 0x014))
#define UART0_THRE     (1u << 5)   /* LSR bit 5: TX holding register empty */
#define UART0_CLK_BIT  (1u << 16)  /* SYSCTRL sys_clk_ctrl UART0 clock */
#define EARLY_LCR_DLAB (1u << 7)   /* Divisor Latch Access Bit */
#define EARLY_LCR_8N1  (0x03u)     /* 8 data bits, no parity, 1 stop bit */
/* Baud rate divisor for 115200 at 64 MHz: 64000000 / (16 * 115200) ≈ 35 */
#define BAUD115200_DIV 35u
#define PINMUX_UART    6u           /* MODE_UART in pin_map nibbles */

static void early_uart_setup(void)
{
    /* Set GPIO16 (bits [3:0]) and GPIO17 (bits [7:4]) to UART mode (6).
     * hosal_uart_init does this via pin_set_mode(); we must do it ourselves
     * in the early diagnostic before the UART driver has been initialised. */
    RT583_PINMAP2 = (RT583_PINMAP2 & ~0xFFu)
                    | (PINMUX_UART << 0)   /* GPIO16 → UART0 */
                    | (PINMUX_UART << 4);  /* GPIO17 → UART0 */

    /* Enable UART0 peripheral clock */
    RT583_SYSCTRL_CLKCTRL |= UART0_CLK_BIT;

    /* Set baud rate: DLAB=1, DLL=35, DLM=0, then DLAB=0, 8N1 */
    UART0_LCR = EARLY_LCR_DLAB | EARLY_LCR_8N1;
    UART0_DLL = BAUD115200_DIV;
    UART0_DLM = 0u;
    UART0_LCR = EARLY_LCR_8N1;
}

static void early_uart_putc(uint8_t c)
{
    for (int i = 0; i < 1000 && !(UART0_LSR & UART0_THRE); i++) {}
    UART0_THR = c;
}

__attribute__((unused))
static void early_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') early_uart_putc('\r');
        early_uart_putc((uint8_t)*s++);
    }
}

static int rt583_soc_init(void)
{
    SystemInit(); /* Required: configures BBPLL to 64 MHz (from librt583_system.a).
                   * Without this the UART peripheral has no clock and
                   * hosal_uart_send_complete spins on LSR.TEMT forever. */
    early_uart_setup(); /* Enable UART0 clock + set 115200 baud @ 64 MHz */

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

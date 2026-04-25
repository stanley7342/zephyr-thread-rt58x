/*
 * RT584 SoC initialisation for Zephyr — minimal secure-mode boot.
 *
 * CPU resets in secure state with SAU at reset value (all-secure). We
 * leave SAU/IDAU alone (no TZ_SAU_Setup), avoid `-mcmse`, and skip the
 * Zephyr TRUSTED_EXECUTION_SECURE framework. This matches the simplest
 * possible CM33 boot flow: secure code accessing peripherals via either
 * 0x4xxxxxxx (non-secure alias, default vendor macros) or 0x5xxxxxxx
 * (secure alias, when CONFIG_*_SECURE_EN is set) — both reachable from
 * secure state.
 *
 * Cherry-pick the bits of upstream systeminit() we actually need for
 * UART output: PMU DCDC tuning, AHB clock to 64 MHz, peripheral clock,
 * UART0 clock gate. Everything else (TZ_SAU_Setup, mpsectorinit,
 * flash_timing_init, RCO calibration) is for use cases we don't have yet.
 *
 * RULE: never call printk from this function — the UART driver init
 * runs at PRE_KERNEL_1 priority 50 (after this), and hosal_uart_send
 * polls LSR.THRE which spins forever if the UART clock is not enabled.
 */

#include <zephyr/init.h>
#include <stdint.h>

extern void     systempmuupdatedcdc(void);
extern uint32_t change_ahb_system_clk(int sys_clk_mode);
extern uint32_t change_peri_clk(int sys_clk_mode);
extern void     enable_perclk(uint32_t clock);
extern void     pin_set_mode(uint32_t pin_number, uint32_t mode);

/* sysctrl.h sys_clk_sel_t values */
#define RT584_SYS_CLK_64MHZ  2
/* sysctrl.h perclk_clk_sel_t values */
#define RT584_PERCLK_32M     0
/* sysctrl.h perclk_id enum: UART0_CLK = 0 */
#define RT584_UART0_CLK_ID   0
/* sysctrl.h pin-mode value: GPIO17 → UART0 TX */
#define RT584_MODE_GPIO      0x00
#define RT584_MODE_UART0_TX  0x01

/* Mirror upstream rt_main.c:pin_mux_init(): force all pins (except
 * GPIO10/11 which are SWD) to MODE_GPIO so none stays in an alt
 * function from a previous boot/test image. */
static void rt584_pin_mux_default(void)
{
    for (uint32_t i = 0; i < 32; i++) {
        if (i == 10 || i == 11) continue;   /* SWD */
        pin_set_mode(i, RT584_MODE_GPIO);
    }
}

/* ── Bare-metal UART0 early diagnostic (non-secure peripheral alias) ─── *
 * SECURE_EN flags are NOT defined now, so vendor's UART0 macro resolves
 * to 0x40012000 (NS alias). We hit that directly here too, before any
 * Zephyr UART driver runs. */
/* rt584 peripheral access lives only at the secure alias 0x5xxxxxxx; the
 * 0x4xxxxxxx mirror reads as 0 / drops writes (verified with GDB). Use
 * the secure alias here too. */
#define EARLY_UART0_BASE  0x50012000UL
#define EARLY_UART0_THR   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x00))
#define EARLY_UART0_IER   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x04))
#define EARLY_UART0_FCR   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x08))
#define EARLY_UART0_LCR   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x0C))
#define EARLY_UART0_LSR   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x14))
#define EARLY_UART0_DLX   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x20))
#define EARLY_UART0_FDL   (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x24))
#define EARLY_UART0_EN    (*(volatile uint32_t *)(EARLY_UART0_BASE + 0x30))

#define EARLY_LSR_THRE    (1u << 5)
#define EARLY_FCR_DEFVAL  (0x02 | 0x04)        /* CLEAR_RCVR | CLEAR_XMIT  */
#define EARLY_LCR_8N1     0x03

static void early_uart_setup(void)
{
    pin_set_mode(17, RT584_MODE_UART0_TX);

    EARLY_UART0_EN  = 0;
    EARLY_UART0_FCR = 0;
    EARLY_UART0_FCR = EARLY_FCR_DEFVAL;
    EARLY_UART0_IER = 0;
    /* 115200 @ 64 MHz peri clock: dlx=34, fdl=6 (BR_FRCT6).
     * baud = 64M / (16*34 + 6) = 116364 ≈ 115200, <1% error. */
    EARLY_UART0_DLX = 34;
    EARLY_UART0_FDL = 6;
    EARLY_UART0_LCR = EARLY_LCR_8N1;
    EARLY_UART0_EN  = 1;
}

static void early_uart_putc(uint8_t c)
{
    for (int i = 0; i < 100000 && !(EARLY_UART0_LSR & EARLY_LSR_THRE); i++) {}
    EARLY_UART0_THR = c;
}

/* Kept around for future bring-up / soc_prep_hook diagnostics — left
 * unused in the steady-state build but preserved so the next stuck-boot
 * investigation can drop a probe here without rewriting register dance. */
__attribute__((unused))
static void early_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') early_uart_putc('\r');
        early_uart_putc((uint8_t)*s++);
    }
}

/* z_prep_c's very first action — runs before .bss zero, .data copy, FPU
 * init, NVIC priorities. We don't print here (would be at the wrong baud
 * since the PLL hasn't been switched to 64 MHz yet); leave this hook as
 * a clean placeholder so any future bring-up can land diagnostics here. */
void soc_prep_hook(void)
{
}

static int rt584_soc_init(void)
{
    /* DCDC tuning before PLL change so 64 MHz BBPLL locks reliably. */
    systempmuupdatedcdc();
    /* 64 MHz AHB; baud divisors and SystemCoreClock assume this. */
    (void)change_ahb_system_clk(RT584_SYS_CLK_64MHZ);
    /* Peripheral clock select (UART baud table calibrated for this). */
    (void)change_peri_clk(RT584_PERCLK_32M);
    /* Default all GPIOs to MODE_GPIO before muxing UART0 below. */
    rt584_pin_mux_default();
    /* Enable UART0 peripheral clock gate (vendor hosal does NOT). */
    enable_perclk(RT584_UART0_CLK_ID);

    /* Set up UART0 at the secure alias 0x50012000 with 115200 @ 64 MHz
     * (DLX=34, FDL=6). The hosal_uart driver init at PRE_KERNEL_1 prio 50
     * will reconfigure the same registers from its own DTS-driven
     * settings; this keeps the early window functional in case anything
     * before then wants to send a byte. */
    early_uart_setup();

    return 0;
}

SYS_INIT(rt584_soc_init, PRE_KERNEL_1, 0);

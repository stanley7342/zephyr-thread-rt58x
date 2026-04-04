/*
 * ot_uart.c — OpenThread UART platform for Zephyr on RT582
 *
 * RX path: interrupt-driven (CONFIG_UART_INTERRUPT_DRIVEN=y)
 *   UART RX IRQ → hosal_rx_callback (ring buf) → uart_rx_irq_cb (ISR ctx)
 *   → assemble line → OT_NOTIFY_ISR → OT task → otCliInputLine
 *
 * TX path: blocking poll-out via uart_poll_out (no TX interrupt needed)
 *
 * RULES:
 *   - uart_rx_irq_cb runs in ISR context — NO printk, NO blocking calls
 *   - Use OT_NOTIFY_ISR (not OT_NOTIFY) from ISR context
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <openthread/config.h>
#include <openthread/platform/logging.h>
#include <openthread/cli.h>

/* otPlatUartSendDone — TX-done callback required by OT UART platform API.
 * Our otPlatUartSend is synchronous (poll_out), so TX is already complete
 * when the function returns; this callback is a no-op. */
void otPlatUartSendDone(void) {}

#include "openthread_port.h"

/* ── UART device ─────────────────────────────────────────────────────────── *
 * Reuse UART0 which is already the Zephyr console (uart_rt582 driver).     */
#define OT_UART_NODE DT_NODELABEL(uart0)

static const struct device *ot_uart_dev;

/* ── RX diagnostic counters — exported for watchdog ─────────────────────── *
 * All written from ISR context; read from task/workqueue context.           *
 * Volatile ensures the compiler doesn't cache the value in a register.      */
volatile uint32_t ot_uart_rx_byte_count; /* total bytes received             */
volatile uint32_t ot_uart_isr_count;     /* times uart_rx_irq_cb was entered */
volatile uint32_t ot_uart_line_count;    /* completed lines sent to OT CLI   */

/* ── Interrupt-driven RX state ───────────────────────────────────────────── *
 * rx_work_buf : partial line being assembled (written by ISR only)          *
 * rx_ready_buf: complete line ready for OT task (written by ISR, read task) *
 * rx_line_ready: true when rx_ready_buf has a new line to process           */
#define RX_LINE_MAXLEN OT_UART_RX_BUFFSIZE

static uint8_t          rx_work_buf[RX_LINE_MAXLEN];
static uint32_t         rx_work_len;
static uint8_t          rx_ready_buf[RX_LINE_MAXLEN];
static volatile bool    rx_line_ready;

/* ── UART RX interrupt callback (ISR context) ────────────────────────────── *
 * Called by hosal_rx_callback after bytes are put into the ring buffer.     *
 * Drains ring buffer, assembles a line, signals OT task on \r or \n.       *
 * Must NOT call printk or any blocking function.                            */
static void uart_rx_irq_cb(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);
    uint8_t c;

    /* ISR context — no printk allowed, use counters only */
    ot_uart_isr_count++;

    while (uart_fifo_read(dev, &c, 1) == 1) {
        ot_uart_rx_byte_count++;

        if (c == '\r' || c == '\n') {
            uart_poll_out(dev, '\r');
            uart_poll_out(dev, '\n');
            if (!rx_line_ready) {
                /* Publish line to task (empty line re-triggers the > prompt) */
                memcpy(rx_ready_buf, rx_work_buf, rx_work_len);
                rx_ready_buf[rx_work_len] = '\0';
                rx_work_len = 0;
                rx_line_ready = true;
                ot_uart_line_count++;
                OT_NOTIFY_ISR(OT_SYSTEM_EVENT_UART_RXD);
            }
        } else if (c == '\b' || c == 0x7f) {
            /* backspace: erase last char */
            if (rx_work_len > 0) {
                rx_work_len--;
                uart_poll_out(dev, '\b');
                uart_poll_out(dev, ' ');
                uart_poll_out(dev, '\b');
            }
        } else if (rx_work_len < RX_LINE_MAXLEN - 1) {
            rx_work_buf[rx_work_len++] = c;
            uart_poll_out(dev, c);  /* echo */
        }
    }
}

/* ── otPlatUartEnable / Disable ──────────────────────────────────────────── */
otError otPlatUartEnable(void)
{
    ot_uart_dev = DEVICE_DT_GET(OT_UART_NODE);
    rx_work_len   = 0;
    rx_line_ready = false;

    printk("[UART] otPlatUartEnable: dev=%p ready=%d\n",
           ot_uart_dev, device_is_ready(ot_uart_dev));

    uart_irq_callback_user_data_set(ot_uart_dev, uart_rx_irq_cb, NULL);
    uart_irq_rx_enable(ot_uart_dev);

    printk("[UART] RX interrupt enabled\n");
    return OT_ERROR_NONE;
}

otError otPlatUartDisable(void)
{
    if (ot_uart_dev) {
        uart_irq_rx_disable(ot_uart_dev);
    }
    return OT_ERROR_NONE;
}

/* ── otPlatUartSend — blocking poll-out via Zephyr UART driver ───────────── */
otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    if (!ot_uart_dev || !device_is_ready(ot_uart_dev)) {
        for (uint16_t i = 0; i < aBufLength; i++) {
            printk("%c", aBuf[i]);
        }
        otPlatUartSendDone();
        return OT_ERROR_NONE;
    }

    for (uint16_t i = 0; i < aBufLength; i++) {
        uart_poll_out(ot_uart_dev, aBuf[i]);
    }
    otPlatUartSendDone();
    return OT_ERROR_NONE;
}

otError otPlatUartFlush(void)
{
    return OT_ERROR_NONE;
}

/* ── ot_uartTask — process completed RX lines ───────────────────────────── *
 * Called from OT task loop. ISR sets rx_line_ready and notifies this task. */
void ot_uartTask(ot_system_event_t sevent)
{
    if ((OT_SYSTEM_EVENT_UART_RXD & sevent) && rx_line_ready) {
        char *line = (char *)rx_ready_buf;
        /* Accept both "ot <cmd>" and "<cmd>" */
        if (line[0] == 'o' && line[1] == 't' &&
            (line[2] == ' ' || line[2] == '\0')) {
            line += (line[2] == ' ') ? 3 : 2;
        }
        rx_line_ready = false;
        otCliInputLine(line);
    }
}

/* ── ot_uartRecieved — external inject path (kept for compatibility) ──────── */
void ot_uartRecieved(uint8_t *rxbuf, uint32_t rxlen)
{
    uint32_t len = rxlen < RX_LINE_MAXLEN - 1 ? rxlen : RX_LINE_MAXLEN - 1;
    if (!rx_line_ready) {
        memcpy(rx_ready_buf, rxbuf, len);
        rx_ready_buf[len] = '\0';
        rx_line_ready = true;
        OT_NOTIFY(OT_SYSTEM_EVENT_UART_RXD);
    }
}

/* ── CLI output callback — routes OT CLI output through otPlatUartSend ─── */
static int cli_output_cb(void *aContext, const char *aFormat, va_list aArguments)
{
    (void)aContext;
    char buf[256];
    int len = vsnprintf(buf, sizeof(buf), aFormat, aArguments);
    otPlatUartSend((const uint8_t *)buf, (uint16_t)strlen(buf));
    return len;
}

/* ── otAppCliInit — app-level CLI initialisation ─────────────────────────── */
void otAppCliInit(otInstance *aInstance)
{
    printk("[CLI] otAppCliInit called, instance=%p\n", aInstance);
    otCliInit(aInstance, cli_output_cb, NULL);
    printk("[CLI] otCliInit done\n");
}

/* ── ot_uartLog — used by ot_logging.c ──────────────────────────────────── */
void ot_uartLog(const char *fmt, va_list argp)
{
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, argp);
    otPlatUartSend((const uint8_t *)buf, (uint16_t)strlen(buf));
}

/*
 * ot_uart.c — OpenThread UART platform for Zephyr on RT582
 *
 * Replaces Rafael SDK ot_uart.c.
 * Uses Zephyr's UART driver (uart_poll_out) instead of hosal_uart_send.
 * FreeRTOS stream_buffer / semphr / task removed entirely.
 * Rafael CLI shell command registration removed — OT commands are fed via
 * the 'ot' shell command registered in src/main.c using Zephyr's shell.
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

/* otPlatUartSendDone is declared in openthread/platform/uart.h in some SDK
 * versions; forward-declare it here to avoid a missing-header error. */
extern void otPlatUartSendDone(void);

#include "openthread_port.h"

/* ── UART device ─────────────────────────────────────────────────────────── *
 * Reuse UART0 which is already the Zephyr console (uart_rt582 driver).     */
#define OT_UART_NODE DT_NODELABEL(uart0)

static const struct device *ot_uart_dev;

/* ── RX byte counter — exported for watchdog diagnostics ────────────────── */
volatile uint32_t ot_uart_rx_byte_count;

/* ── Poll-mode RX line buffer ────────────────────────────────────────────── */
static uint8_t rx_poll_buf[OT_UART_RX_BUFFSIZE];
static uint32_t rx_poll_len;

/* ── Receive buffer (fed by ot_uartRecieved inject path) ────────────────── */
typedef struct {
    uint8_t buf[OT_UART_RX_BUFFSIZE];
} otUart_t;

static otUart_t otUart_var;

/* ── otPlatUartEnable / Disable ──────────────────────────────────────────── */
otError otPlatUartEnable(void)
{
    ot_uart_dev = DEVICE_DT_GET(OT_UART_NODE);
    memset(&otUart_var, 0, sizeof(otUart_var));
    return OT_ERROR_NONE;
}

otError otPlatUartDisable(void)
{
    return OT_ERROR_NONE;
}

/* ── otPlatUartSend — blocking poll-out via Zephyr UART driver ───────────── */
otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    if (!ot_uart_dev || !device_is_ready(ot_uart_dev)) {
        /* Fallback: printk */
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

/* ── ot_uartTask — poll UART RX and process complete lines ──────────────── *
 * Called every OT task loop iteration (≤10 ms interval).                   *
 * Uses uart_poll_in — no CONFIG_UART_INTERRUPT_DRIVEN needed.               *
 * ot_uart_rx_byte_count is exported so the watchdog can show RX activity.  */
void ot_uartTask(ot_system_event_t sevent)
{
    /* ── Poll-mode RX: drain UART FIFO ─────────────────────────────────── */
    if (ot_uart_dev && device_is_ready(ot_uart_dev)) {
        uint8_t c;
        while (uart_poll_in(ot_uart_dev, &c) == 0) {
            ot_uart_rx_byte_count++;
            if (c == '\n' || c == '\r') {
                if (rx_poll_len > 0) {
                    rx_poll_buf[rx_poll_len] = '\0';
                    otCliInputLine((char *)rx_poll_buf);
                    rx_poll_len = 0;
                }
            } else if (rx_poll_len < sizeof(rx_poll_buf) - 1) {
                rx_poll_buf[rx_poll_len++] = c;
            }
        }
    }

    /* ── Inject path (ot_uartRecieved) ─────────────────────────────────── */
    if (OT_SYSTEM_EVENT_UART_RXD & sevent) {
        otCliInputLine((char *)otUart_var.buf);
    }
}

/* ── ot_uartRecieved — called externally to inject a CLI line ────────────── *
 * Used by the Zephyr shell 'ot' command handler in src/main.c.             */
void ot_uartRecieved(uint8_t *rxbuf, uint32_t rxlen)
{
    printk("[CLI] ot_uartRecieved: len=%u buf='%.*s'\n",
           (unsigned)rxlen, (int)rxlen, rxbuf);
    uint32_t len = rxlen < sizeof(otUart_var.buf) - 1
                   ? rxlen : sizeof(otUart_var.buf) - 1;
    memcpy(otUart_var.buf, rxbuf, len);
    otUart_var.buf[len] = '\0';
    OT_NOTIFY(OT_SYSTEM_EVENT_UART_RXD);
}

/* ── otAppCliInit — app-level CLI initialisation ─────────────────────────── *
 * The standard otAppCliInit symbol lives in the SDK's libapp.a (FreeRTOS).  *
 * We provide it here so the app target can call it without that library.    */
void otAppCliInit(otInstance *aInstance)
{
    printk("[CLI] otAppCliInit called, instance=%p\n", aInstance);
    otCliInit(aInstance, NULL, NULL);
    printk("[CLI] otCliInit done\n");
}

/* ── ot_uartLog — used by ot_logging.c ──────────────────────────────────── */
void ot_uartLog(const char *fmt, va_list argp)
{
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, argp);
    otPlatUartSend((const uint8_t *)buf, (uint16_t)strlen(buf));
}

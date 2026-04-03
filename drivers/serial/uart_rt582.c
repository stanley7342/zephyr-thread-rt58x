/*
 * Zephyr UART driver for Rafael RT582
 *
 * Supports polling TX/RX and interrupt-driven RX/TX.
 *
 * TX interrupt simulation: uart_irq_tx_enable() submits a k_work item that
 * calls the registered IRQ callback from the system workqueue.  The callback
 * drains the shell's TX ring-buffer via uart_fifo_fill(), then disables TX IRQ.
 *
 * RX interrupts: the HOSAL rx_cb fires when bytes arrive; we buffer them in a
 * small ring buffer and call the IRQ callback so the shell wakes up.
 */

#define DT_DRV_COMPAT rafael_rt582_uart

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "hosal_uart.h"

#define UART_DBG(fmt, ...) printk("[UART] " fmt "\n", ##__VA_ARGS__)

/* ── Small RX ring buffer ────────────────────────────────────────────────── */
#define RX_BUF_SIZE 64

typedef struct {
    uint8_t buf[RX_BUF_SIZE];
    uint8_t head;
    uint8_t tail;
} rx_ring_t;

static inline void rx_ring_put(rx_ring_t *r, uint8_t c)
{
    uint8_t next = (uint8_t)((r->tail + 1U) % RX_BUF_SIZE);
    if (next != r->head) {
        r->buf[r->tail] = c;
        r->tail = next;
    }
}

static inline int rx_ring_get(rx_ring_t *r, uint8_t *c)
{
    if (r->head == r->tail) {
        return -1;
    }
    *c = r->buf[r->head];
    r->head = (uint8_t)((r->head + 1U) % RX_BUF_SIZE);
    return 0;
}

static inline bool rx_ring_empty(const rx_ring_t *r)
{
    return r->head == r->tail;
}

/* ── Per-instance driver data ────────────────────────────────────────────── */

struct uart_rt582_data {
    hosal_uart_dev_t hosal_dev;
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    uart_irq_callback_user_data_t irq_cb;
    void        *irq_cb_data;
    rx_ring_t    rx_ring;
    bool         rx_irq_enabled;
    bool         tx_irq_enabled;
    struct k_work tx_work;
    const struct device *self;
#endif
};

struct uart_rt582_cfg {
    uint32_t base;
    uint32_t baud_rate;
    uint8_t  uart_id;
    uint8_t  tx_pin;
    uint8_t  rx_pin;
};

/* ── Polling API ─────────────────────────────────────────────────────────── */

static int uart_rt582_poll_in(const struct device *dev, unsigned char *c)
{
    struct uart_rt582_data *data = dev->data;

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    if (data->rx_irq_enabled) {
        return rx_ring_get(&data->rx_ring, c);
    }
#endif
    return (hosal_uart_receive(&data->hosal_dev, c, 1) == 1) ? 0 : -1;
}

static void uart_rt582_poll_out(const struct device *dev, unsigned char c)
{
    struct uart_rt582_data *data = dev->data;
    hosal_uart_send(&data->hosal_dev, &c, 1);
    hosal_uart_send_complete(&data->hosal_dev);
}

/* ── Interrupt-driven API ────────────────────────────────────────────────── */

#ifdef CONFIG_UART_INTERRUPT_DRIVEN

/* Called from HOSAL ISR when RX bytes arrive */
static int hosal_rx_callback(void *p_arg)
{
    const struct device    *dev  = (const struct device *)p_arg;
    struct uart_rt582_data *data = dev->data;
    uint8_t c;
    int count = 0;

    while (hosal_uart_receive(&data->hosal_dev, &c, 1) == 1) {
        rx_ring_put(&data->rx_ring, c);
        count++;
    }
    UART_DBG("RX ISR: %d bytes", count);

    if (data->irq_cb) {
        data->irq_cb(dev, data->irq_cb_data);
    }
    return 0;
}

/* TX work handler — runs in system workqueue, simulates TX-ready IRQ */
static void tx_work_handler(struct k_work *work)
{
    struct uart_rt582_data *data =
        CONTAINER_OF(work, struct uart_rt582_data, tx_work);

    UART_DBG("TX work: irq_cb=%p tx_en=%d", data->irq_cb, data->tx_irq_enabled);
    if (data->tx_irq_enabled && data->irq_cb) {
        data->irq_cb(data->self, data->irq_cb_data);
    }
}

static int uart_rt582_fifo_fill(const struct device *dev,
                                const uint8_t *tx_data, int len)
{
    struct uart_rt582_data *data = dev->data;
    hosal_uart_send(&data->hosal_dev, tx_data, len);
    hosal_uart_send_complete(&data->hosal_dev);
    return len;
}

static int uart_rt582_fifo_read(const struct device *dev,
                                uint8_t *rx_data, const int size)
{
    struct uart_rt582_data *data = dev->data;
    int read = 0;

    while (read < size && rx_ring_get(&data->rx_ring, &rx_data[read]) == 0) {
        read++;
    }
    return read;
}

static void uart_rt582_irq_tx_enable(const struct device *dev)
{
    struct uart_rt582_data *data = dev->data;
    UART_DBG("TX enable");
    data->tx_irq_enabled = true;
    k_work_submit(&data->tx_work);
}

static void uart_rt582_irq_tx_disable(const struct device *dev)
{
    struct uart_rt582_data *data = dev->data;
    data->tx_irq_enabled = false;
}

static int uart_rt582_irq_tx_ready(const struct device *dev)
{
    struct uart_rt582_data *data = dev->data;
    return data->tx_irq_enabled ? 1 : 0;
}

static int uart_rt582_irq_tx_complete(const struct device *dev)
{
    (void)dev;
    return 1;
}

static void uart_rt582_irq_rx_enable(const struct device *dev)
{
    struct uart_rt582_data *data = dev->data;
    UART_DBG("RX enable (already=%d)", data->rx_irq_enabled);
    if (!data->rx_irq_enabled) {
        hosal_uart_mode_t mode = HOSAL_UART_MODE_INT_RX;
        int rc = hosal_uart_ioctl(&data->hosal_dev, HOSAL_UART_MODE_SET, &mode);
        UART_DBG("  ioctl MODE_SET rc=%d", rc);
        rc = hosal_uart_callback_set(&data->hosal_dev, HOSAL_UART_RX_CALLBACK,
                                     hosal_rx_callback, (void *)dev);
        UART_DBG("  callback_set rc=%d", rc);
        data->rx_irq_enabled = true;
    }
}

static void uart_rt582_irq_rx_disable(const struct device *dev)
{
    struct uart_rt582_data *data = dev->data;
    if (data->rx_irq_enabled) {
        hosal_uart_callback_set(&data->hosal_dev, HOSAL_UART_RX_CALLBACK,
                                NULL, NULL);
        hosal_uart_mode_t mode = HOSAL_UART_MODE_POLL;
        hosal_uart_ioctl(&data->hosal_dev, HOSAL_UART_MODE_SET, &mode);
        data->rx_irq_enabled = false;
    }
}

static int uart_rt582_irq_rx_ready(const struct device *dev)
{
    struct uart_rt582_data *data = dev->data;
    return rx_ring_empty(&data->rx_ring) ? 0 : 1;
}

static int uart_rt582_irq_is_pending(const struct device *dev)
{
    return uart_rt582_irq_rx_ready(dev);
}

static int uart_rt582_irq_update(const struct device *dev)
{
    (void)dev;
    return 1;
}

static void uart_rt582_irq_callback_set(const struct device *dev,
                                        uart_irq_callback_user_data_t cb,
                                        void *user_data)
{
    struct uart_rt582_data *data = dev->data;
    UART_DBG("IRQ callback set: cb=%p", cb);
    data->irq_cb      = cb;
    data->irq_cb_data = user_data;
}

#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

/* ── Driver API table ────────────────────────────────────────────────────── */

static const struct uart_driver_api uart_rt582_api = {
    .poll_in  = uart_rt582_poll_in,
    .poll_out = uart_rt582_poll_out,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    .fifo_fill        = uart_rt582_fifo_fill,
    .fifo_read        = uart_rt582_fifo_read,
    .irq_tx_enable    = uart_rt582_irq_tx_enable,
    .irq_tx_disable   = uart_rt582_irq_tx_disable,
    .irq_tx_ready     = uart_rt582_irq_tx_ready,
    .irq_tx_complete  = uart_rt582_irq_tx_complete,
    .irq_rx_enable    = uart_rt582_irq_rx_enable,
    .irq_rx_disable   = uart_rt582_irq_rx_disable,
    .irq_rx_ready     = uart_rt582_irq_rx_ready,
    .irq_is_pending   = uart_rt582_irq_is_pending,
    .irq_update       = uart_rt582_irq_update,
    .irq_callback_set = uart_rt582_irq_callback_set,
#endif
};

/* ── Driver initialisation ───────────────────────────────────────────────── */

static hosal_uart_baudrate_t baud_to_hosal(uint32_t baud)
{
    switch (baud) {
    case   2400: return UART_BAUDRATE_Baud2400;
    case   9600: return UART_BAUDRATE_Baud9600;
    case  19200: return UART_BAUDRATE_Baud19200;
    case  38400: return UART_BAUDRATE_Baud38400;
    case  57600: return UART_BAUDRATE_Baud57600;
    case 115200: return UART_BAUDRATE_Baud115200;
    case 230400: return UART_BAUDRATE_Baud230400;
    case 500000: return UART_BAUDRATE_Baud500000;
    default:     return UART_BAUDRATE_Baud115200;
    }
}

static int uart_rt582_init(const struct device *dev)
{
    const struct uart_rt582_cfg *cfg  = dev->config;
    struct uart_rt582_data      *data = dev->data;

    data->hosal_dev.port              = cfg->uart_id;
    data->hosal_dev.config.uart_id    = cfg->uart_id;
    data->hosal_dev.config.tx_pin     = cfg->tx_pin;
    data->hosal_dev.config.rx_pin     = cfg->rx_pin;
    data->hosal_dev.config.cts_pin    = 255;
    data->hosal_dev.config.rts_pin    = 255;
    data->hosal_dev.config.baud_rate  = baud_to_hosal(cfg->baud_rate);
    data->hosal_dev.config.data_width = UART_DATA_BITS_8;
    data->hosal_dev.config.parity     = UART_PARITY_NONE;
    data->hosal_dev.config.stop_bits  = UART_STOPBIT_ONE;
    data->hosal_dev.tx_cb             = NULL;
    data->hosal_dev.rx_cb             = NULL;

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    data->self           = dev;
    data->rx_irq_enabled = false;
    data->tx_irq_enabled = false;
    k_work_init(&data->tx_work, tx_work_handler);
#endif

    int rc = hosal_uart_init(&data->hosal_dev);
    UART_DBG("init uart_id=%d rc=%d", cfg->uart_id, rc);
    return rc;
}

/* ── DTS instance expansion ──────────────────────────────────────────────── */

#define UART_RT582_DEVICE(inst)                                              \
    static struct uart_rt582_data uart_rt582_data_##inst;                    \
                                                                             \
    static const struct uart_rt582_cfg uart_rt582_cfg_##inst = {             \
        .base      = DT_INST_REG_ADDR(inst),                                 \
        .baud_rate = DT_INST_PROP(inst, current_speed),                      \
        .uart_id   = DT_INST_PROP(inst, uart_id),                            \
        .tx_pin    = DT_INST_PROP_OR(inst, tx_pin, 255),                     \
        .rx_pin    = DT_INST_PROP_OR(inst, rx_pin, 255),                     \
    };                                                                       \
                                                                             \
    DEVICE_DT_INST_DEFINE(inst,                                              \
                          uart_rt582_init,                                   \
                          NULL,                                              \
                          &uart_rt582_data_##inst,                           \
                          &uart_rt582_cfg_##inst,                            \
                          PRE_KERNEL_1,                                      \
                          CONFIG_SERIAL_INIT_PRIORITY,                       \
                          &uart_rt582_api);

DT_INST_FOREACH_STATUS_OKAY(UART_RT582_DEVICE)

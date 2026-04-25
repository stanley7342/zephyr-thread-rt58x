/*
 * Zephyr UART driver for Rafael RT584 (Cortex-M33)
 *
 * Mirrors drivers/serial/uart_rt583.c almost verbatim — same hosal_uart API,
 * same do-not-printk-in-ISR rule, same IRQ_CONNECT requirement. The two
 * differences are:
 *
 *   1. DT compatible is rafael,rt584-uart (uart_id property unchanged).
 *   2. baud_to_hosal() maps to the rt584 UART_BAUDRATE_Baud* enum values,
 *      which are different from rt583 because rt584's hosal_uart baud
 *      register is a 17-bit (DLX | fractional) layout clocked at 32 MHz.
 */

#define DT_DRV_COMPAT rafael_rt584_uart

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include "hosal_uart.h"

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
        __DMB();
        r->tail = next;
    }
}

static inline int rx_ring_get(rx_ring_t *r, uint8_t *c)
{
    if (r->head == r->tail) return -1;
    __DMB();
    *c = r->buf[r->head];
    r->head = (uint8_t)((r->head + 1U) % RX_BUF_SIZE);
    return 0;
}

static inline bool rx_ring_empty(const rx_ring_t *r)
{
    return r->head == r->tail;
}

struct uart_rt584_data {
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

struct uart_rt584_cfg {
    uint32_t base;
    uint32_t baud_rate;
    uint8_t  uart_id;
    uint8_t  tx_pin;
    uint8_t  rx_pin;
};

static int uart_rt584_poll_in(const struct device *dev, unsigned char *c)
{
    struct uart_rt584_data *data = dev->data;
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    if (data->rx_irq_enabled) return rx_ring_get(&data->rx_ring, c);
#endif
    return (hosal_uart_receive(&data->hosal_dev, c, 1) == 1) ? 0 : -1;
}

static void uart_rt584_poll_out(const struct device *dev, unsigned char c)
{
    struct uart_rt584_data *data = dev->data;
    hosal_uart_send(&data->hosal_dev, &c, 1);
}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN

static int hosal_rx_callback(void *p_arg)
{
    const struct device    *dev  = (const struct device *)p_arg;
    struct uart_rt584_data *data = dev->data;
    uint8_t c;

    while (hosal_uart_receive(&data->hosal_dev, &c, 1) == 1)
        rx_ring_put(&data->rx_ring, c);

    if (data->irq_cb)
        data->irq_cb(dev, data->irq_cb_data);
    return 0;
}

static void tx_work_handler(struct k_work *work)
{
    struct uart_rt584_data *data =
        CONTAINER_OF(work, struct uart_rt584_data, tx_work);
    if (data->tx_irq_enabled && data->irq_cb)
        data->irq_cb(data->self, data->irq_cb_data);
}

static int uart_rt584_fifo_fill(const struct device *dev,
                                const uint8_t *tx_data, int len)
{
    struct uart_rt584_data *data = dev->data;
    hosal_uart_send(&data->hosal_dev, tx_data, len);
    return len;
}

static int uart_rt584_fifo_read(const struct device *dev,
                                uint8_t *rx_data, const int size)
{
    struct uart_rt584_data *data = dev->data;
    int read = 0;
    while (read < size && rx_ring_get(&data->rx_ring, &rx_data[read]) == 0)
        read++;
    return read;
}

static void uart_rt584_irq_tx_enable(const struct device *dev)
{
    struct uart_rt584_data *data = dev->data;
    data->tx_irq_enabled = true;
    k_work_submit(&data->tx_work);
}

static void uart_rt584_irq_tx_disable(const struct device *dev)
{
    struct uart_rt584_data *data = dev->data;
    data->tx_irq_enabled = false;
}

static int uart_rt584_irq_tx_ready(const struct device *dev)
{
    struct uart_rt584_data *data = dev->data;
    return data->tx_irq_enabled ? 1 : 0;
}

static int uart_rt584_irq_tx_complete(const struct device *dev)
{
    (void)dev; return 1;
}

static void uart_rt584_irq_rx_enable(const struct device *dev)
{
    struct uart_rt584_data *data = dev->data;
    if (!data->rx_irq_enabled) {
        hosal_uart_ioctl(&data->hosal_dev, HOSAL_UART_MODE_SET,
                         (void *)(uintptr_t)HOSAL_UART_MODE_INT_RX);
        hosal_uart_callback_set(&data->hosal_dev, HOSAL_UART_RX_CALLBACK,
                                hosal_rx_callback, (void *)dev);
        data->rx_irq_enabled = true;
    }
}

static void uart_rt584_irq_rx_disable(const struct device *dev)
{
    struct uart_rt584_data *data = dev->data;
    if (data->rx_irq_enabled) {
        hosal_uart_callback_set(&data->hosal_dev, HOSAL_UART_RX_CALLBACK,
                                NULL, NULL);
        hosal_uart_ioctl(&data->hosal_dev, HOSAL_UART_MODE_SET,
                         (void *)(uintptr_t)HOSAL_UART_MODE_POLL);
        data->rx_irq_enabled = false;
    }
}

static int uart_rt584_irq_rx_ready(const struct device *dev)
{
    struct uart_rt584_data *data = dev->data;
    return rx_ring_empty(&data->rx_ring) ? 0 : 1;
}

static int uart_rt584_irq_is_pending(const struct device *dev)
{
    return uart_rt584_irq_rx_ready(dev);
}

static int uart_rt584_irq_update(const struct device *dev)
{
    (void)dev; return 1;
}

static void uart_rt584_irq_callback_set(const struct device *dev,
                                        uart_irq_callback_user_data_t cb,
                                        void *user_data)
{
    struct uart_rt584_data *data = dev->data;
    data->irq_cb      = cb;
    data->irq_cb_data = user_data;
}

#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

static const struct uart_driver_api uart_rt584_api = {
    .poll_in  = uart_rt584_poll_in,
    .poll_out = uart_rt584_poll_out,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    .fifo_fill        = uart_rt584_fifo_fill,
    .fifo_read        = uart_rt584_fifo_read,
    .irq_tx_enable    = uart_rt584_irq_tx_enable,
    .irq_tx_disable   = uart_rt584_irq_tx_disable,
    .irq_tx_ready     = uart_rt584_irq_tx_ready,
    .irq_tx_complete  = uart_rt584_irq_tx_complete,
    .irq_rx_enable    = uart_rt584_irq_rx_enable,
    .irq_rx_disable   = uart_rt584_irq_rx_disable,
    .irq_rx_ready     = uart_rt584_irq_rx_ready,
    .irq_is_pending   = uart_rt584_irq_is_pending,
    .irq_update       = uart_rt584_irq_update,
    .irq_callback_set = uart_rt584_irq_callback_set,
#endif
};

/* rt584 hosal_uart baud enum: 32 MHz peripheral clock, 17-bit DLX|fractional.
 * The names collide with rt583's different-valued enums if both are visible,
 * but hosal_uart.h is only picked up when compiling this TU so we get the
 * correct rt584 values. */
static uint32_t baud_to_hosal(uint32_t baud)
{
    switch (baud) {
    case   9600: return UART_BAUDRATE_Baud9600;
    case  19200: return UART_BAUDRATE_Baud19200;
    case  38400: return UART_BAUDRATE_Baud38400;
    case  57600: return UART_BAUDRATE_Baud57600;
    case 115200: return UART_BAUDRATE_Baud115200;
    case 230400: return UART_BAUDRATE_Baud230400;
    case 500000: return UART_BAUDRATE_Baud500000;
    case 1000000: return UART_BAUDRATE_Baud1000000;
    case 2000000: return UART_BAUDRATE_Baud2000000;
    default:     return UART_BAUDRATE_Baud115200;
    }
}

extern void uart0_handler(void);
extern void uart1_handler(void);

static void uart_rt584_isr(const void *arg)
{
    const struct device *dev = arg;
    const struct uart_rt584_cfg *cfg = dev->config;
    switch (cfg->uart_id) {
    case 0: uart0_handler(); break;
    case 1: uart1_handler(); break;
    default: break;
    }
}

#define UART_RT584_DEVICE(inst)                                              \
    static struct uart_rt584_data uart_rt584_data_##inst;                    \
                                                                             \
    static const struct uart_rt584_cfg uart_rt584_cfg_##inst = {             \
        .base      = DT_INST_REG_ADDR(inst),                                 \
        .baud_rate = DT_INST_PROP(inst, current_speed),                      \
        .uart_id   = DT_INST_PROP(inst, uart_id),                            \
        .tx_pin    = DT_INST_PROP_OR(inst, tx_pin, 255),                     \
        .rx_pin    = DT_INST_PROP_OR(inst, rx_pin, 255),                     \
    };                                                                       \
                                                                             \
    static int uart_rt584_init_##inst(const struct device *dev)              \
    {                                                                        \
        const struct uart_rt584_cfg *cfg  = dev->config;                     \
        struct uart_rt584_data      *data = dev->data;                       \
                                                                             \
        data->hosal_dev.port              = cfg->uart_id;                    \
        data->hosal_dev.config.uart_id    = cfg->uart_id;                    \
        data->hosal_dev.config.tx_pin     = cfg->tx_pin;                     \
        data->hosal_dev.config.rx_pin     = cfg->rx_pin;                     \
        data->hosal_dev.config.cts_pin    = 255;                             \
        data->hosal_dev.config.rts_pin    = 255;                             \
        data->hosal_dev.config.baud_rate  = baud_to_hosal(cfg->baud_rate);   \
        data->hosal_dev.config.data_width = UART_DATA_BITS_8;                \
        data->hosal_dev.config.parity     = UART_PARITY_NONE;                \
        data->hosal_dev.config.stop_bits  = UART_STOPBIT_ONE;                \
        data->hosal_dev.tx_cb             = NULL;                            \
        data->hosal_dev.rx_cb             = NULL;                            \
                                                                             \
        IF_ENABLED(CONFIG_UART_INTERRUPT_DRIVEN, (                           \
            data->self           = dev;                                      \
            data->rx_irq_enabled = false;                                    \
            data->tx_irq_enabled = false;                                    \
            k_work_init(&data->tx_work, tx_work_handler);                    \
        ))                                                                   \
                                                                             \
        hosal_uart_init(&data->hosal_dev);                                   \
                                                                             \
        IRQ_CONNECT(DT_INST_IRQN(inst),                                      \
                    DT_INST_IRQ(inst, priority),                             \
                    uart_rt584_isr,                                          \
                    DEVICE_DT_INST_GET(inst), 0);                            \
        return 0;                                                            \
    }                                                                        \
                                                                             \
    DEVICE_DT_INST_DEFINE(inst,                                              \
                          uart_rt584_init_##inst,                            \
                          NULL,                                              \
                          &uart_rt584_data_##inst,                           \
                          &uart_rt584_cfg_##inst,                            \
                          PRE_KERNEL_1,                                      \
                          CONFIG_SERIAL_INIT_PRIORITY,                       \
                          &uart_rt584_api);

DT_INST_FOREACH_STATUS_OKAY(UART_RT584_DEVICE)

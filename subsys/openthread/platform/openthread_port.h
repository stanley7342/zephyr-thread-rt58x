/*
 * OpenThread platform port header — Zephyr RTOS edition
 *
 * Drop-in replacement for the Rafael SDK openthread_port.h.
 * All FreeRTOS primitives are replaced with Zephyr equivalents:
 *
 *   taskENTER_CRITICAL()           → irq_lock()  (stored in __ot_irq_key)
 *   taskEXIT_CRITICAL()            → irq_unlock(__ot_irq_key)
 *   xTaskNotifyGive / ulTaskNotifyTake → k_sem_give/take(&ot_task_sem)
 *   xPortIsInsideInterrupt()        → k_is_in_isr()
 *   xSemaphoreTake/Give (mutex)     → k_mutex_lock/unlock(&ot_ext_lock)
 */

#ifndef OPENTHREAD_PORT_H
#define OPENTHREAD_PORT_H

#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <openthread/instance.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Stack / priority configuration ─────────────────────────────────────── */
#ifndef CONFIG_OPENTHREAD_TASK_SIZE
#define CONFIG_OPENTHREAD_TASK_SIZE 4096
#endif

#ifndef OT_TASK_PRIORITY
#define OT_TASK_PRIORITY 5
#endif

#ifndef OT_UART_RX_BUFFSIZE
#define OT_UART_RX_BUFFSIZE 384
#endif

/* ── Event bitmask ───────────────────────────────────────────────────────── */
typedef enum _ot_system_event {
    OT_SYSTEM_EVENT_NONE = 0,

    OT_SYSTEM_EVENT_OT_TASKLET            = 0x00000001,

    OT_SYSTEM_EVENT_ALARM_MS_EXPIRED      = 0x00000002,
    OT_SYSTEM_EVENT_ALARM_US_EXPIRED      = 0x00000004,
    OT_SYSTEM_EVENT_ALARM_ALL_MASK        = OT_SYSTEM_EVENT_ALARM_MS_EXPIRED
                                           | OT_SYSTEM_EVENT_ALARM_US_EXPIRED,

    OT_SYSTEM_EVENT_UART_TXR              = 0x00000010,
    OT_SYSTEM_EVENT_UART_TXD              = 0x00000020,
    OT_SYSTEM_EVENT_UART_RXD              = 0x00000040,
    OT_SYSETM_EVENT_UART_ALL_MASK         = OT_SYSTEM_EVENT_UART_TXR
                                           | OT_SYSTEM_EVENT_UART_TXD
                                           | OT_SYSTEM_EVENT_UART_RXD,

    OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ = 0x00000100,
    OT_SYSTEM_EVENT_RADIO_TX_ERROR        = 0x00000200,
    OT_SYSTEM_EVENT_RADIO_TX_ACKED        = 0x00000400,
    OT_SYSTEM_EVENT_RADIO_TX_NO_ACK       = 0x00000800,
    OT_SYSTEM_EVENT_RADIO_TX_CCA_FAIL     = 0x00001000,
    OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK     = OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ
                                           | OT_SYSTEM_EVENT_RADIO_TX_ERROR
                                           | OT_SYSTEM_EVENT_RADIO_TX_ACKED
                                           | OT_SYSTEM_EVENT_RADIO_TX_NO_ACK
                                           | OT_SYSTEM_EVENT_RADIO_TX_CCA_FAIL,

    OT_SYSTEM_EVENT_RADIO_RX_NO_BUFF     = 0x00002000,
    OT_SYSTEM_EVENT_RADIO_RX_DONE        = 0x00004000,
    OT_SYSTEM_EVENT_RADIO_RX_CRC_FIALED  = 0x00008000,
    OT_SYSTEM_EVENT_RADIO_RX_ALL_MASK    = OT_SYSTEM_EVENT_RADIO_RX_NO_BUFF
                                          | OT_SYSTEM_EVENT_RADIO_RX_DONE
                                          | OT_SYSTEM_EVENT_RADIO_RX_CRC_FIALED,
    OT_SYSTEM_EVENT_RADIO_ALL_MASK       = OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK
                                          | OT_SYSTEM_EVENT_RADIO_RX_ALL_MASK,

    OT_SYSTEM_EVENT_OTA_HANDLER          = 0x00010000,
    OT_SYSTEM_EVENT_APP                  = 0xff000000,
    OT_SYSTEM_EVENT_ALL                  = 0xffffffff,
} ot_system_event_t;

extern ot_system_event_t ot_system_event_var;

/* ── Critical-section helpers ────────────────────────────────────────────── *
 * irq_lock/unlock disable/restore IRQs on Cortex-M (like FreeRTOS              *
 * taskENTER/EXIT_CRITICAL).  We store the key in a module-level variable       *
 * because the SDK macros are always used in matched pairs in the same scope.   */
extern unsigned int __ot_irq_key;

static ALWAYS_INLINE void _ot_enter_critical(void) {
    __ot_irq_key = irq_lock();
}
static ALWAYS_INLINE void _ot_exit_critical(void) {
    irq_unlock(__ot_irq_key);
}

#define OT_ENTER_CRITICAL()     _ot_enter_critical()
#define OT_EXIT_CRITICAL()      _ot_exit_critical()
/* ISR variants: IRQs are already disabled inside an ISR on Cortex-M */
#define OT_ENTER_CRITICAL_ISR() ((void)0)
#define OT_EXIT_CRITICAL_ISR(x) ((void)(x))

/* ── Task-notification (wake the OT thread) ──────────────────────────────── */
extern struct k_sem ot_task_sem;

/* Signal from task context */
static ALWAYS_INLINE void _ot_notify(ot_system_event_t bit) {
    OT_ENTER_CRITICAL();
    ot_system_event_var |= bit;
    OT_EXIT_CRITICAL();
    k_sem_give(&ot_task_sem);
}

/* Signal from ISR context (k_sem_give is ISR-safe in Zephyr) */
static ALWAYS_INLINE void _ot_notify_isr(ot_system_event_t bit) {
    ot_system_event_var |= bit;   /* atomic not strictly needed: IRQs are off */
    k_sem_give(&ot_task_sem);
}

#define OT_NOTIFY(ebit)     _ot_notify((ot_system_event_t)(ebit))
#define OT_NOTIFY_ISR(ebit) _ot_notify_isr((ot_system_event_t)(ebit))

#define OT_GET_NOTIFY(ebit) \
    do { \
        OT_ENTER_CRITICAL(); \
        (ebit) = ot_system_event_var; \
        ot_system_event_var = OT_SYSTEM_EVENT_NONE; \
        OT_EXIT_CRITICAL(); \
    } while (0)

/* ── Thread-safe access wrappers ─────────────────────────────────────────── */
extern struct k_mutex ot_ext_lock;

#define OT_THREAD_SAFE(...) \
    do { \
        k_mutex_lock(&ot_ext_lock, K_FOREVER); \
        do { __VA_ARGS__; } while (0); \
        k_mutex_unlock(&ot_ext_lock); \
    } while (0)

#define OT_THREAD_SAFE_RET(ret, ...) \
    do { \
        k_mutex_lock(&ot_ext_lock, K_FOREVER); \
        (ret) = (__VA_ARGS__); \
        k_mutex_unlock(&ot_ext_lock); \
    } while (0)

/* ── CLI uart lock stubs (no-op for Zephyr) ──────────────────────────────── */
#define OT_CLI_UART_OUTPUT_LOCK()
#define OT_CLI_UART_OUTPUT_UNLOCK()

/* ── App-layer notification helpers ─────────────────────────────────────── */
#define OT_APP_NOTIFY_ISR(ebit) \
    _ot_notify_isr((ot_system_event_t)((ebit) & OT_SYSTEM_EVENT_APP))

#define OT_APP_NOTIFY(ebit) \
    _ot_notify((ot_system_event_t)((ebit) & OT_SYSTEM_EVENT_APP))

/* ── Public API ──────────────────────────────────────────────────────────── */
void otrStackInit(void);
void otrStart(void);
void otrInitUser(otInstance *instance);
otInstance *otrGetInstance(void);

void ot_uartTask(ot_system_event_t sevent);
void ot_uartRecieved(uint8_t *rxbuf, uint32_t rxlen);
void ot_uartSetFd(int fd);
void ot_uartLog(const char *fmt, va_list argp);

void ot_alarmInit(void);
void ot_alarmTask(ot_system_event_t sevent);

void ot_entropy_init(void);

void ot_radioInit(void);
void ot_radio_short_addr_ctrl(uint8_t ctrl_type, uint8_t *short_addr);
void ot_radio_extend_addr_ctrl(uint8_t ctrl_type, uint8_t *extend_addr);
void ot_radioTask(ot_system_event_t trxEvent);

extern void otAppCliInit(otInstance *aInstance);

void otSysEventSignalPending(void);
void otrLock(void);
void otrUnlock(void);

uint64_t otPlatTimeGet(void);

#ifdef __cplusplus
}
#endif

#endif /* OPENTHREAD_PORT_H */

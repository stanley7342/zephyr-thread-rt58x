/*
 * ot_zephyr.c — OpenThread task management for Zephyr RTOS
 *
 * Replaces Rafael SDK ot_freertos.c.
 * All FreeRTOS primitives are mapped to Zephyr equivalents:
 *
 *   xSemaphoreCreateMutexStatic → K_MUTEX_DEFINE (ot_ext_lock)
 *   xTaskCreate                 → k_thread_create
 *   ulTaskNotifyTake(pdFALSE,1) → k_sem_take(&ot_task_sem, K_MSEC(10))
 *   xTaskNotifyGive             → k_sem_give(&ot_task_sem)
 *   vTaskNotifyGiveFromISR      → k_sem_give (ISR-safe in Zephyr)
 *   xPortIsInsideInterrupt      → k_is_in_isr()
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>

#include "openthread_port.h"
#include "tprintk.h"

/* Forward declaration — defined in ot_uart.c */
extern otError otPlatUartEnable(void);

/* ── Globals shared with openthread_port.h macros ───────────────────────── */
ot_system_event_t ot_system_event_var = OT_SYSTEM_EVENT_NONE;

/* irq_lock() key — used by OT_ENTER_CRITICAL / OT_EXIT_CRITICAL */
unsigned int __ot_irq_key;

/* Mutex protecting OpenThread API calls from multiple threads */
K_MUTEX_DEFINE(ot_ext_lock);

/* Binary semaphore used to wake the OT thread (replaces task notification) */
K_SEM_DEFINE(ot_task_sem, 0, 1);

/* OT thread stack and control block */
#define OT_THREAD_STACK_SIZE CONFIG_OPENTHREAD_TASK_SIZE
K_THREAD_STACK_DEFINE(ot_thread_stack, OT_THREAD_STACK_SIZE);
static struct k_thread ot_thread_data;

static otInstance *ot_instance;

/* ── Weak hook: application code runs inside the OT task loop ───────────── */
__attribute__((weak)) void otrAppProcess(ot_system_event_t sevent) {}
__attribute__((weak)) void otrInitUser(otInstance *instance) {}

/* ── otTaskletsSignalPending ─────────────────────────────────────────────── */
void otTaskletsSignalPending(otInstance *aInstance)
{
    if (aInstance) {
        OT_NOTIFY(OT_SYSTEM_EVENT_OT_TASKLET);
    }
}

/* ── otSysEventSignalPending ─────────────────────────────────────────────── *
 * Called by radio/alarm ISR callbacks to wake the OT thread.               *
 * k_sem_give() is ISR-safe in Zephyr — no need to distinguish ISR vs task. */
void otSysEventSignalPending(void)
{
    k_sem_give(&ot_task_sem);
}

/* ── API lock/unlock (for callers outside the OT thread) ────────────────── */
void otrLock(void)
{
    k_mutex_lock(&ot_ext_lock, K_FOREVER);
}

void otrUnlock(void)
{
    k_mutex_unlock(&ot_ext_lock);
}

/* ── Accessor ────────────────────────────────────────────────────────────── */
otInstance *otrGetInstance(void)
{
    return ot_instance;
}

/* ── Driver poll (called every iteration of the OT task loop) ───────────── */
void otSysProcessDrivers(otInstance *aInstance)
{
    ot_system_event_t sevent = OT_SYSTEM_EVENT_NONE;

    OT_GET_NOTIFY(sevent);
    ot_alarmTask(sevent);
    otrAppProcess(sevent);
    ot_uartTask(sevent);
    ot_radioTask(sevent);
}

/* ── OT thread entry ─────────────────────────────────────────────────────── */
static void otrStackTask(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("[OT] task started");
    OT_THREAD_SAFE(
        otPlatUartEnable();
        printk("[OT] uart enable done");
        ot_entropy_init();
        printk("[OT] entropy init done");
        ot_alarmInit();
        printk("[OT] alarm init done");
        ot_radioInit();
        printk("[OT] radio init done");
        ot_instance = otInstanceInitSingle();
        printk("[OT] instance=%p", ot_instance);
        assert(ot_instance);
#if OPENTHREAD_ENABLE_DIAG
        otDiagInit(ot_instance);
#endif
        otrInitUser(ot_instance);
        printk("[OT] otrInitUser done");
    );

    while (true) {
        /*
         * Wait up to 10 ms for a notification, then unconditionally
         * process tasklets and drivers.  This mirrors the original
         * ulTaskNotifyTake(pdFALSE, 1) pattern but uses a sane timeout.
         */
        k_sem_take(&ot_task_sem, K_MSEC(10));

        k_mutex_lock(&ot_ext_lock, K_FOREVER);
        otTaskletsProcess(ot_instance);
        otSysProcessDrivers(ot_instance);
        k_mutex_unlock(&ot_ext_lock);
    }

    otInstanceFinalize(ot_instance);
    ot_instance = NULL;
}

/* ── otrStart: create and launch the OT thread ───────────────────────────── */
void otrStart(void)
{
    k_thread_create(&ot_thread_data,
                    ot_thread_stack,
                    K_THREAD_STACK_SIZEOF(ot_thread_stack),
                    otrStackTask,
                    NULL, NULL, NULL,
                    OT_TASK_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&ot_thread_data, "ot-thread");
}

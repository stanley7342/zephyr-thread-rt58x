/*
 * freertos_shim.c — Minimal FreeRTOS compatibility shim for Zephyr
 *
 * The pre-built liblmac15p4.a was compiled against FreeRTOS and calls a
 * small set of FreeRTOS queue/timing APIs.  We provide Zephyr-backed
 * implementations of exactly those symbols so the binary links cleanly.
 *
 * Symbols satisfied:
 *   vTaskDelay, xPortIsInsideInterrupt,
 *   xQueueGenericCreateStatic, xQueueSemaphoreTake, xQueueGenericSend,
 *   xTaskGetTickCount, xTaskGetTickCountFromISR
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/kernel.h>

/* ── FreeRTOS type aliases ────────────────────────────────────────────────── */
typedef uint32_t TickType_t;
typedef uint32_t UBaseType_t;
typedef int32_t  BaseType_t;
typedef void    *QueueHandle_t;

#define portMAX_DELAY            ((TickType_t)0xFFFFFFFFUL)
#define pdTRUE                   ((BaseType_t)1)
#define pdFALSE                  ((BaseType_t)0)

/* Matches queueQUEUE_TYPE_BINARY_SEMAPHORE in FreeRTOS */
#define QUEUE_TYPE_BINARY_SEM    3U

/* ── Internal queue/semaphore wrapper ────────────────────────────────────── */
typedef struct {
    bool is_sem;
    union {
        struct k_sem  sem;
        struct k_msgq msgq;
    };
} shim_q_t;

#define SHIM_MAX_QUEUES 8
static shim_q_t shim_pool[SHIM_MAX_QUEUES];
static int      shim_count;

/* ── Internal helper: allocate and initialise a queue/semaphore ─────────── */
static shim_q_t *shim_alloc_queue(UBaseType_t uxQueueLength,
                                   UBaseType_t uxItemSize,
                                   uint8_t    *pucStorage,
                                   uint8_t     ucQueueType)
{
    if (shim_count >= SHIM_MAX_QUEUES) {
        return NULL;
    }
    shim_q_t *q = &shim_pool[shim_count++];
    memset(q, 0, sizeof(*q));

    if (uxItemSize == 0) {
        q->is_sem = true;
        k_sem_init(&q->sem,
                   (ucQueueType == QUEUE_TYPE_BINARY_SEM) ? 0U : uxQueueLength,
                   uxQueueLength);
    } else {
        q->is_sem = false;
        k_msgq_init(&q->msgq, (char *)pucStorage,
                    (size_t)uxItemSize, (uint32_t)uxQueueLength);
    }
    return q;
}

/* ── xQueueGenericCreate (dynamic) ──────────────────────────────────────── *
 * hosal_rf.c uses the dynamic form. We allocate message storage from heap. */
QueueHandle_t xQueueGenericCreate(UBaseType_t uxQueueLength,
                                   UBaseType_t uxItemSize,
                                   uint8_t     ucQueueType)
{
    uint8_t *storage = NULL;
    if (uxItemSize > 0) {
        storage = malloc(uxQueueLength * uxItemSize);
        if (!storage) {
            return NULL;
        }
    }
    return (QueueHandle_t)shim_alloc_queue(uxQueueLength, uxItemSize,
                                            storage, ucQueueType);
}

/* ── xQueueGenericCreateStatic ──────────────────────────────────────────── *
 * lmac15p4 calls this at init to create its TX semaphore and TX queue.     *
 * uxItemSize == 0 → semaphore; uxItemSize > 0 → message queue.            *
 * pucQueueStorage is the caller-supplied circular buffer for queue items.  */
QueueHandle_t xQueueGenericCreateStatic(UBaseType_t    uxQueueLength,
                                        UBaseType_t    uxItemSize,
                                        uint8_t       *pucQueueStorage,
                                        void          *pxStaticQueue,
                                        uint8_t        ucQueueType)
{
    if (shim_count >= SHIM_MAX_QUEUES) {
        return NULL;
    }
    return (QueueHandle_t)shim_alloc_queue(uxQueueLength, uxItemSize,
                                            pucQueueStorage, ucQueueType);
}

/* ── xQueueSemaphoreTake ─────────────────────────────────────────────────── */
BaseType_t xQueueSemaphoreTake(QueueHandle_t xQueue, TickType_t xTicksToWait)
{
    shim_q_t *q = (shim_q_t *)xQueue;
    k_timeout_t t = (xTicksToWait == portMAX_DELAY)
                    ? K_FOREVER : K_TICKS(xTicksToWait);
    return (k_sem_take(&q->sem, t) == 0) ? pdTRUE : pdFALSE;
}

/* ── xQueueGenericSend ───────────────────────────────────────────────────── *
 * xCopyPosition: 0 = back of queue (normal), 1 = front (prepend).         */
BaseType_t xQueueGenericSend(QueueHandle_t  xQueue,
                              const void    *pvItemToQueue,
                              TickType_t     xTicksToWait,
                              BaseType_t     xCopyPosition)
{
    shim_q_t *q = (shim_q_t *)xQueue;
    k_timeout_t t;

    if (xTicksToWait == portMAX_DELAY) {
        t = K_FOREVER;
    } else if (xTicksToWait == 0) {
        t = K_NO_WAIT;
    } else {
        t = K_TICKS(xTicksToWait);
    }

    if (q->is_sem) {
        k_sem_give(&q->sem);
        return pdTRUE;
    }
    return (k_msgq_put(&q->msgq, pvItemToQueue, t) == 0) ? pdTRUE : pdFALSE;
}

/* ── vTaskDelay ──────────────────────────────────────────────────────────── */
void vTaskDelay(TickType_t xTicksToDelay)
{
    k_sleep(K_TICKS(xTicksToDelay));
}

/* ── xPortIsInsideInterrupt ──────────────────────────────────────────────── */
BaseType_t xPortIsInsideInterrupt(void)
{
    return k_is_in_isr() ? pdTRUE : pdFALSE;
}

/* ── xTaskGetTickCount / xTaskGetTickCountFromISR ────────────────────────── */
TickType_t xTaskGetTickCount(void)
{
    return (TickType_t)k_uptime_ticks();
}

TickType_t xTaskGetTickCountFromISR(void)
{
    return (TickType_t)k_uptime_ticks();
}

/* ── FreeRTOS memory (pvPortMalloc / vPortFree) ──────────────────────────── *
 * librt582_hosal.a and the crypto alt objects call these.                   *
 * Map to the newlib heap that Zephyr's newlib integration provides.         */
#include <stdlib.h>
void *pvPortMalloc(size_t xWantedSize) { return malloc(xWantedSize); }
void  vPortFree(void *pv)              { free(pv); }

/* ── xQueueReceive ───────────────────────────────────────────────────────── */
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer,
                          TickType_t xTicksToWait)
{
    shim_q_t *q = (shim_q_t *)xQueue;
    k_timeout_t t = (xTicksToWait == portMAX_DELAY)
                    ? K_FOREVER : K_TICKS(xTicksToWait);
    return (k_msgq_get(&q->msgq, pvBuffer, t) == 0) ? pdTRUE : pdFALSE;
}

/* ── xTaskCreate ─────────────────────────────────────────────────────────── *
 * hosal_rf creates a background event-processing task.                     *
 * usStackDepth is in 32-bit words; multiply by 4 to get bytes.             */
#define SHIM_TASK_STACK_BYTES(words) ((words) * sizeof(uint32_t))

BaseType_t xTaskCreate(void (*pvTaskCode)(void *),
                        const char    *pcName,
                        uint16_t       usStackDepth,
                        void          *pvParameters,
                        uint32_t       uxPriority,
                        void         **pxCreatedTask)
{
    size_t stack_bytes = SHIM_TASK_STACK_BYTES(usStackDepth);
    k_thread_stack_t *stack = k_thread_stack_alloc(stack_bytes, 0);
    if (!stack) {
        return pdFALSE;
    }
    struct k_thread *thread = malloc(sizeof(struct k_thread));
    if (!thread) {
        k_thread_stack_free(stack);
        return pdFALSE;
    }
    /* Map FreeRTOS priority (higher = more important) to Zephyr priority.
     * FreeRTOS: 0 = lowest, configMAX_PRIORITIES-1 = highest.
     * Zephyr coop: -1 (highest) to -CONFIG_NUM_COOP_PRIORITIES (lowest). */
    int zephyr_prio = -(int)uxPriority - 1;
    if (zephyr_prio < -(CONFIG_NUM_COOP_PRIORITIES - 1)) {
        zephyr_prio = -(CONFIG_NUM_COOP_PRIORITIES - 1);
    }
    k_tid_t tid = k_thread_create(thread, stack, stack_bytes,
                                   (k_thread_entry_t)pvTaskCode,
                                   pvParameters, NULL, NULL,
                                   zephyr_prio, 0, K_NO_WAIT);
    k_thread_name_set(tid, pcName ? pcName : "freertos_task");
    if (pxCreatedTask) {
        *pxCreatedTask = (void *)tid;
    }
    return pdTRUE;
}

/* ── Task notification semaphore ─────────────────────────────────────────── *
 * hosal_rf's __rf_proc task calls ulTaskNotifyTake() to block, and the RF  *
 * ISR calls vTaskGenericNotifyGiveFromISR() to wake it.  We use a single   *
 * binary semaphore covering the one task that uses this pattern.           */
static K_SEM_DEFINE(task_notify_sem, 0, 1);

/* ── ulTaskGenericNotifyTake ─────────────────────────────────────────────── *
 * Used by __rf_proc to block waiting for RF events.                        */
uint32_t ulTaskGenericNotifyTake(uint32_t uxIndexToWaitOn,
                                  int32_t  xClearCountOnExit,
                                  uint32_t xTicksToWait)
{
    (void)uxIndexToWaitOn;
    (void)xClearCountOnExit;
    k_timeout_t t = (xTicksToWait == portMAX_DELAY)
                    ? K_FOREVER : K_TICKS(xTicksToWait);
    return (k_sem_take(&task_notify_sem, t) == 0) ? 1U : 0U;
}

/* ── vTaskGenericNotifyGiveFromISR ───────────────────────────────────────── *
 * RF ISR calls this to wake __rf_proc.                                     */
void vTaskGenericNotifyGiveFromISR(void *xTaskToNotify,
                                    uint32_t uxIndexToNotify,
                                    int32_t *pxHigherPriorityTaskWoken)
{
    (void)xTaskToNotify;
    (void)uxIndexToNotify;
    k_sem_give(&task_notify_sem);
    if (pxHigherPriorityTaskWoken) {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }
}

/* ── FreeRTOS critical section (sysfun.c in librt582_driver.a) ───────────── *
 * sysfun.c's leave_critical_section() calls vPortExitCritical() on EVERY   *
 * decrement (not just when the driver-level counter reaches zero), relying   *
 * on FreeRTOS's own uxCriticalNesting to gate the actual IRQ re-enable.    *
 * We must replicate that behaviour: only call irq_lock/unlock at the first  *
 * and last nesting level respectively, otherwise BASEPRI gets stuck at      *
 * 0x20 after any nested enter/exit pair and CommSubsystem IRQs are masked   *
 * permanently.                                                               */
static unsigned int _port_irq_key;
static uint32_t     _port_critical_nest;

void vPortEnterCritical(void)
{
    if (_port_critical_nest == 0) {
        _port_irq_key = irq_lock();
    }
    _port_critical_nest++;
}

void vPortExitCritical(void)
{
    if (_port_critical_nest == 0) {
        return;  /* safety: unbalanced calls */
    }
    _port_critical_nest--;
    if (_port_critical_nest == 0) {
        irq_unlock(_port_irq_key);
    }
}

/* ── xTaskGetSchedulerState ─────────────────────────────────────────────── *
 * sysfun.c calls this to decide whether to use critical sections.          *
 * Return taskSCHEDULER_RUNNING (2) so the critical-section path is taken.  */
BaseType_t xTaskGetSchedulerState(void) { return (BaseType_t)2; }

/* ── FreeRTOS software timers ────────────────────────────────────────────── *
 * librt569-rf.a and librt582_driver.a reference xTimerCreate and           *
 * xTimerGenericCommandFromTask in object files that handle optional RF      *
 * power-compensation features not used by our port.  Provide stubs so the  *
 * linker is satisfied if those objects are ever pulled in.                  */
void *xTimerCreate(const char *pcTimerName, uint32_t xTimerPeriodInTicks,
                   uint32_t uxAutoReload, void *pvTimerID,
                   void (*pxCallbackFunction)(void *))
{
    ARG_UNUSED(pcTimerName);
    ARG_UNUSED(xTimerPeriodInTicks);
    ARG_UNUSED(uxAutoReload);
    ARG_UNUSED(pvTimerID);
    ARG_UNUSED(pxCallbackFunction);
    return NULL;  /* not supported */
}

BaseType_t xTimerGenericCommandFromTask(void *xTimer, BaseType_t xCommandID,
                                         TickType_t xOptionalValue,
                                         BaseType_t *pxHigherPriorityTaskWoken,
                                         TickType_t xTicksToWait)
{
    ARG_UNUSED(xTimer);
    ARG_UNUSED(xCommandID);
    ARG_UNUSED(xOptionalValue);
    ARG_UNUSED(pxHigherPriorityTaskWoken);
    ARG_UNUSED(xTicksToWait);
    return pdFALSE;
}

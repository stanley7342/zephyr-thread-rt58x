/*
 * rt583_compat.c — Thin Zephyr wrappers for pre-built library compatibility
 *
 * The pre-built .a libraries (librt583_driver.a, librt583_system.a, and the
 * OpenThread stack libs) were compiled against FreeRTOS and reference a small
 * set of FreeRTOS symbols at link time.  This file provides Zephyr-backed
 * implementations of exactly those symbols so the binaries link cleanly.
 *
 * Symbols satisfied:
 *   vTaskDelay
 *   pvPortMalloc, vPortFree
 *   vPortEnterCritical, vPortExitCritical
 *   xTaskGetSchedulerState
 *   xTimerCreate, xTimerGenericCommandFromTask  (stubs — not exercised at runtime)
 *
 * Internal helpers (called from hosal_rf_zephyr.c):
 *   _crit_unlock, _crit_relock
 */

#include <stdint.h>
#include <stdlib.h>
#include <zephyr/kernel.h>

typedef uint32_t TickType_t;
typedef int32_t  BaseType_t;

#define pdFALSE  ((BaseType_t)0)

/* ── Scheduler-lock critical section ─────────────────────────────────────── *
 *                                                                            *
 * Uses k_sched_lock() so hardware IRQs (COMM_SUBSYSTEM, SysTick) stay       *
 * enabled while preventing thread preemption inside SDK critical sections.  *
 *                                                                            *
 * _crit_unlock / _crit_relock: called from hosal_rf_zephyr.c to temporarily  *
 * release the lock before blocking on Zephyr primitives.                    */
static uint32_t _port_critical_nest;

uint32_t _crit_unlock(void)
{
    uint32_t nest = _port_critical_nest;
    if (nest > 0) {
        _port_critical_nest = 0;
        k_sched_unlock();
    }
    return nest;
}

void _crit_relock(uint32_t nest)
{
    if (nest > 0) {
        k_sched_lock();
        _port_critical_nest = nest;
    }
}

/* ── vTaskDelay ──────────────────────────────────────────────────────────── */
void vTaskDelay(TickType_t xTicksToDelay)
{
    uint32_t nest = _crit_unlock();
    k_sleep(K_TICKS(xTicksToDelay));
    _crit_relock(nest);
}

/* ── Memory allocation ───────────────────────────────────────────────────── */
void *pvPortMalloc(size_t xWantedSize) { return malloc(xWantedSize); }
void  vPortFree(void *pv)              { free(pv); }

/* ── Critical section ────────────────────────────────────────────────────── */
void vPortEnterCritical(void)
{
    if (_port_critical_nest == 0) {
        k_sched_lock();
    }
    _port_critical_nest++;
}

void vPortExitCritical(void)
{
    if (_port_critical_nest == 0) {
        return;
    }
    _port_critical_nest--;
    if (_port_critical_nest == 0) {
        k_sched_unlock();
    }
}

/* ── xTaskGetSchedulerState ──────────────────────────────────────────────── *
 * Return taskSCHEDULER_RUNNING (2) so SDK critical-section paths activate.  */
BaseType_t xTaskGetSchedulerState(void) { return (BaseType_t)2; }

/* ── Software timer stubs ────────────────────────────────────────────────── *
 * librt583_driver.a references these for optional RF power-compensation     *
 * features not used by this port.                                           */
void *xTimerCreate(const char *pcTimerName, uint32_t xTimerPeriodInTicks,
                   uint32_t uxAutoReload, void *pvTimerID,
                   void (*pxCallbackFunction)(void *))
{
    ARG_UNUSED(pcTimerName);
    ARG_UNUSED(xTimerPeriodInTicks);
    ARG_UNUSED(uxAutoReload);
    ARG_UNUSED(pvTimerID);
    ARG_UNUSED(pxCallbackFunction);
    return NULL;
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

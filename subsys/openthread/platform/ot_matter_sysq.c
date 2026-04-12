/*
 * ot_matter_sysq.c — OpenThread event dispatch for the RT583 Matter build.
 *
 * In the Matter (CHIP) build, OT is managed by Zephyr's openthread module
 * (openthread_work_q work queue thread) rather than the custom otrStackTask
 * thread in ot_zephyr.c.  This file replaces ot_zephyr.c for that build:
 *
 *   - Provides the globals that openthread_port.h exposes as extern
 *     (ot_system_event_var, __ot_irq_key, ot_task_sem, ot_ext_lock).
 *   - Provides otrGetInstance() / ot_set_instance() so that ot_alarm.c can
 *     call otPlatAlarmMilliFired(otrGetInstance()) once the OT instance is
 *     available (set by AppTask after openthread_init() returns).
 *   - Overrides otSysProcessDrivers() so that Zephyr's openthread_process()
 *     work queue handler dispatches alarm/radio events via our RT583 platform
 *     handlers instead of Zephyr's generic platformAlarmProcess/RadioProcess.
 *   - Does NOT define otTaskletsSignalPending() or otSysEventSignalPending(),
 *     letting Zephyr's openthread.c versions be used: they submit
 *     openthread_work to openthread_work_q, which is what we want for Matter.
 *
 * OT_NOTIFY / OT_NOTIFY_ISR in openthread_port.h call otSysEventSignalPending()
 * (not k_sem_give(&ot_task_sem) directly), so alarm/radio ISRs automatically
 * trigger the work queue when events occur.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <openthread/instance.h>

#include "openthread_port.h"

/* ── Globals declared extern in openthread_port.h ───────────────────────── */

/* Event accumulator written atomically by alarm/radio ISRs via OT_NOTIFY_ISR,
 * and read+cleared by OT_GET_NOTIFY in otSysProcessDrivers. */
ot_system_event_t ot_system_event_var = OT_SYSTEM_EVENT_NONE;

/* IRQ lock key for OT_ENTER_CRITICAL / OT_EXIT_CRITICAL pairs. */
unsigned int __ot_irq_key;

/* Dummy semaphore — OT_NOTIFY gave this before our openthread_port.h change;
 * keep the symbol to satisfy any remaining extern references in linked objects.
 * It is not taken by any thread in the Matter build. */
K_SEM_DEFINE(ot_task_sem, 0, 1);

/* Mutex used by OT_THREAD_SAFE wrappers — kept for completeness. */
K_MUTEX_DEFINE(ot_ext_lock);

/* ── OT instance tracking ────────────────────────────────────────────────── */

static otInstance *s_ot_instance;

/**
 * ot_set_instance — called by AppTask after openthread_init() so that
 * ot_alarmTask() can pass the instance to otPlatAlarmMilliFired().
 */
void ot_set_instance(otInstance *inst)
{
    s_ot_instance = inst;
}

/**
 * otrGetInstance — returns the current OT instance.
 * Returns NULL before ot_set_instance() is called; ot_alarmTask() checks
 * the event mask before calling this, so a NULL return during early boot
 * is benign (no alarm events fire until OT has started its state machine).
 */
otInstance *otrGetInstance(void)
{
    return s_ot_instance;
}

/* ── No-op stubs for the CLI/thread-management API ──────────────────────── */

/* otrStart: not called in Matter — OT processing runs via openthread_work_q. */
void otrStart(void) { }

/* otrLock/Unlock: Matter uses openthread_mutex_lock/unlock from Zephyr's
 * openthread.c instead of these.  Provide no-ops to satisfy any lingering
 * call sites in linked SDK object files. */
void otrLock(void)   { k_mutex_lock(&ot_ext_lock, K_FOREVER); }
void otrUnlock(void) { k_mutex_unlock(&ot_ext_lock); }

/* otrInitUser / otrAppProcess: weak hooks for application code; no-ops here. */
__attribute__((weak)) void otrInitUser(otInstance *instance) { (void)instance; }
__attribute__((weak)) void otrAppProcess(ot_system_event_t sevent) { (void)sevent; }

/* otrStackInit: legacy init shim, not needed in Matter. */
void otrStackInit(void) { }

/* ── otSysProcessDrivers override ───────────────────────────────────────── */

/**
 * otSysProcessDrivers — dispatches accumulated alarm and radio events.
 *
 * Called from Zephyr's openthread_process() work queue handler after
 * otTaskletsProcess().  Reads and clears ot_system_event_var (set by
 * alarm/radio ISRs via OT_NOTIFY_ISR), then calls the RT583 platform event
 * handlers.  This overrides Zephyr's generic version in
 * modules/openthread/platform/platform.c (--allow-multiple-definition; our
 * app sources link before libopenthread_platform.a).
 */
void otSysProcessDrivers(otInstance *aInstance)
{
    ot_system_event_t sevent = OT_SYSTEM_EVENT_NONE;

    OT_GET_NOTIFY(sevent);

    /* Periodically confirm the OT work queue is alive by logging a brief
     * summary every 512 invocations (~5–10 s at normal idle rate).
     * Also log any non-zero event mask immediately so we can see radio/alarm
     * activity after ConnectNetwork enables Thread with a new dataset. */
    static uint32_t s_call_count;
    s_call_count++;
    if (sevent != OT_SYSTEM_EVENT_NONE) {
        if (sevent & (OT_SYSTEM_EVENT_RADIO_RX_DONE | OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK)) {
            /* Radio events are frequent — only print a dot every 64 to keep
             * the log readable without flooding it.                           */
            static uint32_t s_radio_ev;
            if ((++s_radio_ev & 0x3F) == 1) {
                printk("[OT] radio ev 0x%x (n=%u)\n", (unsigned)sevent, (unsigned)s_call_count);
            }
        } else {
            printk("[OT] sys ev 0x%x (n=%u)\n", (unsigned)sevent, (unsigned)s_call_count);
        }
    } else if ((s_call_count & 0x1FF) == 0) {
        printk("[OT] alive n=%u\n", (unsigned)s_call_count);
    }

    ot_alarmTask(sevent);       /* alarm-milli / alarm-micro event dispatch */
    ot_uartTask(sevent);        /* no-op stub in rt583_otsys.c              */
    ot_radioTask(sevent);       /* TX-done / RX-done event dispatch          */
}

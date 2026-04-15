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
#include <openthread/thread.h>
#include <openthread/ip6.h>
#include <openthread/srp_client.h>
#include <openthread/netdata.h>

#include "openthread_port.h"
#include "hosal_rf.h"

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

/* ── OT instance tracking (forward declaration for diag) ────────────────── */

static otInstance *s_ot_instance;

/* ── Periodic RF diagnostic dump ────────────────────────────────────────── */

static void rf_diag_work_fn(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(rf_diag_work, rf_diag_work_fn);

static const char *ot_role_str(otDeviceRole role)
{
    switch (role) {
    case OT_DEVICE_ROLE_DISABLED: return "disabled";
    case OT_DEVICE_ROLE_DETACHED: return "detached";
    case OT_DEVICE_ROLE_CHILD:    return "child";
    case OT_DEVICE_ROLE_ROUTER:   return "router";
    case OT_DEVICE_ROLE_LEADER:   return "leader";
    default:                      return "?";
    }
}

static void print_ipv6_addr(const otIp6Address *addr)
{
    printk("%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
           (unsigned)((addr->mFields.m8[0] << 8) | addr->mFields.m8[1]),
           (unsigned)((addr->mFields.m8[2] << 8) | addr->mFields.m8[3]),
           (unsigned)((addr->mFields.m8[4] << 8) | addr->mFields.m8[5]),
           (unsigned)((addr->mFields.m8[6] << 8) | addr->mFields.m8[7]),
           (unsigned)((addr->mFields.m8[8] << 8) | addr->mFields.m8[9]),
           (unsigned)((addr->mFields.m8[10] << 8) | addr->mFields.m8[11]),
           (unsigned)((addr->mFields.m8[12] << 8) | addr->mFields.m8[13]),
           (unsigned)((addr->mFields.m8[14] << 8) | addr->mFields.m8[15]));
}

static void rf_diag_work_fn(struct k_work *work)
{
    hosal_rf_dump_diag();
    if (s_ot_instance) {
        otDeviceRole role = otThreadGetDeviceRole(s_ot_instance);
        printk("[OT] role=%s rloc=0x%04x part=0x%08x\n",
               ot_role_str(role),
               otThreadGetRloc16(s_ot_instance),
               (unsigned)otThreadGetPartitionId(s_ot_instance));

        /* Dump all IPv6 addresses — key for SRP/mDNS debugging */
        if (role != OT_DEVICE_ROLE_DISABLED && role != OT_DEVICE_ROLE_DETACHED) {
            const otNetifAddress *addr = otIp6GetUnicastAddresses(s_ot_instance);
            for (; addr; addr = addr->mNext) {
                printk("[OT]   addr=");
                print_ipv6_addr(&addr->mAddress);
                printk("/%d%s%s%s\n", addr->mPrefixLength,
                       addr->mMeshLocal ? " mesh" : "",
                       addr->mPreferred ? " pref" : "",
                       addr->mValid ? " valid" : "");
            }

            /* SRP client host info */
            const otSrpClientHostInfo *host = otSrpClientGetHostInfo(s_ot_instance);
            if (host && host->mName) {
                printk("[SRP] host=%s state=%d addrs=%u\n",
                       host->mName, host->mState,
                       (unsigned)host->mNumAddresses);
                for (uint8_t i = 0; i < host->mNumAddresses; i++) {
                    printk("[SRP]   addr=");
                    print_ipv6_addr(&host->mAddresses[i]);
                    printk("\n");
                }
            }

            /* SRP client services */
            const otSrpClientService *svc = otSrpClientGetServices(s_ot_instance);
            for (; svc; svc = svc->mNext) {
                printk("[SRP] svc=%s.%s port=%u state=%d\n",
                       svc->mInstanceName, svc->mName,
                       svc->mPort, svc->mState);
            }
        }
    }
    k_work_reschedule(&rf_diag_work, K_SECONDS(10));
}

/* ── OT instance management ─────────────────────────────────────────────── */

/**
 * ot_set_instance — called by AppTask after openthread_init() so that
 * ot_alarmTask() can pass the instance to otPlatAlarmMilliFired().
 * Also starts the periodic RF diagnostic dump.
 */
void ot_set_instance(otInstance *inst)
{
    s_ot_instance = inst;
    /* First dump at 5 s, then every 10 s, to capture early-boot RUCI state. */
    k_work_schedule(&rf_diag_work, K_SECONDS(5));
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

    ot_alarmTask(sevent);       /* alarm-milli / alarm-micro event dispatch */
    ot_uartTask(sevent);        /* no-op stub in rt583_otsys.c              */
    ot_radioTask(sevent);       /* TX-done / RX-done event dispatch          */
}

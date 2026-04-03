/*
 * ot_alarm.c — OpenThread alarm/timer platform for Zephyr on RT582
 *
 * FreeRTOS → Zephyr replacements:
 *   xTimerCreate / xTimerChangePeriod → K_TIMER_DEFINE + k_timer_start()
 *   xTimerStop                        → k_timer_stop()
 *   xTimerIsTimerActive               → k_timer_remaining_get() != 0
 *   xTaskGetTickCount() * portTICK_RATE_MS → k_uptime_get_32()
 *   pdMS_TO_TICKS(ms)                 → ms  (k_timer takes ms directly)
 *
 * The microsecond timer keeps direct RT582 TIMER3 register access.
 * The IRQ is registered via Zephyr's IRQ_CONNECT (replacing
 * timer_callback_register() + NVIC_EnableIRQ()).
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/irq.h>

#include <openthread/config.h>
#include <openthread/link.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>
#include <openthread/thread.h>

#include "openthread_port.h"
#include "code_utils.h"

/* Rafael RT582 hardware timer registers (from mcu.h) */
#include "mcu.h"
#include "lmac15p4.h"

/* ── Millisecond timer (k_timer) ─────────────────────────────────────────── */
static void _ms_timer_expiry(struct k_timer *t);
K_TIMER_DEFINE(ot_ms_timer, _ms_timer_expiry, NULL);

static void _ms_timer_expiry(struct k_timer *t)
{
    OT_NOTIFY(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
}

/* ── Microsecond timer (RT582 TIMER3 hardware registers) ─────────────────── */
#if (OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE == 1)

#define ALARM_TIMER_TICK_TO_MICRO_SEC(n)  ((n) * 25UL)
#define ALARM_MICRO_SEC_TO_TIMER_TICK(n)  \
    (((n) % 25UL > 13UL) ? ((n) / 25UL + 1UL) : ((n) / 25UL))
#define ALRAM_TIMER_COUNTER_COMPARE1(n1, n2) \
    (((n1) - (n2)) <= (0xFFFFFFFFUL / 2) ? ((n1) - (n2)) : 0)
#define ALRAM_TIMER_COUNTER_COMPARE2(n1, n2) \
    (((n2) - (n1)) > (0xFFFFFFFFUL / 2) ? (0xFFFFFFFFUL - (n2) + (n1)) : 0)
#define ALRAM_TIMER_COUNTER_CHECK(n1, n2) \
    ((n1) > (n2) ? ALRAM_TIMER_COUNTER_COMPARE1(n1, n2) \
                 : ALRAM_TIMER_COUNTER_COMPARE2(n1, n2))

/* Timer3 IRQ number from mcu.h (IRQn_Type: Timer3_IRQn = 17) */
#define RT582_TIMER3_IRQ   17
#define RT582_TIMER3_PRIO   2

static void _us_timer_isr(const void *unused)
{
    ARG_UNUSED(unused);
    /* Clear interrupt — TIMER3->clear write-1-to-clear */
    TIMER3->clear = 1;
    TIMER3->control.bit.int_enable = 0;
    TIMER3->control.bit.en = 0;

    OT_NOTIFY_ISR(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
}
#endif /* OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE */

/* ── ot_alarmInit ────────────────────────────────────────────────────────── */
void ot_alarmInit(void)
{
    /* ms timer is a static k_timer — no runtime init needed */

#if (OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE == 1)
    /* Configure TIMER3 for free-run at 40 MHz / prescale-1 */
    irq_disable(RT582_TIMER3_IRQ);

    TIMER3->load = 0;
    TIMER3->clear = 1;
    TIMER3->control.reg = 0;
    TIMER3->control.bit.prescale = 0; /* TIMER_PRESCALE_1 */
    TIMER3->control.bit.mode = 0;     /* TIMER_FREERUN_MODE */
    TIMER3->control.bit.en = 0;

    IRQ_CONNECT(RT582_TIMER3_IRQ, RT582_TIMER3_PRIO, _us_timer_isr, NULL, 0);
    irq_enable(RT582_TIMER3_IRQ);
#endif
}

/* ── ot_alarmTask ────────────────────────────────────────────────────────── */
void ot_alarmTask(ot_system_event_t sevent)
{
    if (!(OT_SYSTEM_EVENT_ALARM_ALL_MASK & sevent)) {
        return;
    }
    if (OT_SYSTEM_EVENT_ALARM_MS_EXPIRED & sevent) {
        otPlatAlarmMilliFired(otrGetInstance());
    }
#if (OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE == 1)
    if (OT_SYSTEM_EVENT_ALARM_US_EXPIRED & sevent) {
        otPlatAlarmMicroFired(otrGetInstance());
    }
#endif
}

/* ── Crystal accuracy ────────────────────────────────────────────────────── */
uint32_t otPlatTimeGetXtalAccuracy(void)
{
    return SystemCoreClock;
}

/* ── Millisecond alarm ───────────────────────────────────────────────────── */
void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    OT_UNUSED_VARIABLE(aInstance);

    uint32_t now = otPlatAlarmMilliGetNow();
    uint32_t elapsed = now - aT0;

    if (elapsed >= aDt) {
        /* Already expired */
        OT_NOTIFY(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
        return;
    }

    uint32_t remaining_ms = aDt - elapsed;
    k_timer_start(&ot_ms_timer, K_MSEC(remaining_ms), K_NO_WAIT);
}

void otPlatAlarmMilliStop(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    k_timer_stop(&ot_ms_timer);
}

uint32_t otPlatAlarmMilliGetNow(void)
{
    return (uint32_t)k_uptime_get_32();
}

/* ── Microsecond alarm ───────────────────────────────────────────────────── */
#if (OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE == 1)

uint32_t otPlatAlarmMicroGetNow(void)
{
    uint32_t rtc = 0;
    lmac15p4_rtc_time_read(&rtc);
    return rtc;
}

void otPlatAlarmMicroStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    OT_UNUSED_VARIABLE(aInstance);

    uint32_t target = aT0 + aDt;
    uint32_t now    = otPlatTimeGet();
    uint32_t remaining = ALRAM_TIMER_COUNTER_CHECK(target, now);

    if (remaining > 3000) {
        TIMER3->load = ALARM_MICRO_SEC_TO_TIMER_TICK(remaining);
        TIMER3->clear = 1;
        TIMER3->control.bit.int_enable = 1;
        TIMER3->control.bit.en = 1;
    } else {
        OT_NOTIFY(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
    }
}

void otPlatAlarmMicroStop(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    TIMER3->control.bit.en = 0;
    TIMER3->control.bit.int_enable = 0;
    TIMER3->clear = 1;
}

#endif /* OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE */

/* ── 64-bit platform time (µs) ───────────────────────────────────────────── */
uint64_t otPlatTimeGet(void)
{
    static uint32_t timerWraps = 0;
    static uint32_t prev32 = 0;
    uint32_t now32;

#if (OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE == 1)
    now32 = otPlatAlarmMicroGetNow();
#else
    now32 = otPlatAlarmMilliGetNow();
#endif

    if (now32 < prev32) {
        timerWraps++;
    }
    prev32 = now32;

    return ((uint64_t)timerWraps << 32) | now32;
}

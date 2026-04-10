/*
 * RT583 OT system platform overrides.
 *
 * Zephyr's modules/openthread/platform/radio.c provides platformRadioInit()
 * which calls radio_api->get_capabilities() and panics if IEEE802154_HW_TX_RX_ACK
 * is not set.  Our RT583 IEEE 802.15.4 stub is a compile-time placeholder only;
 * the real 802.15.4 radio is managed by ot_radio.c via lmac15p4.
 *
 * We override platformRadioInit() and platformRadioProcess() here with no-ops.
 * This file is compiled into app/libapp.a which is linked before
 * modules/openthread/platform/libopenthread_platform.a; with
 * -Wl,--allow-multiple-definition in effect the first definition wins.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <openthread/instance.h>

/**
 * platformRadioInit — no-op.
 *
 * Real radio initialisation is performed by ot_radioInit() in ot_radio.c,
 * which is called from AppTask::Init() after hosal_rf_init() has prepared the
 * RF MCU.  Zephyr's radio.c version would panic because the RT583 IEEE 802.15.4
 * stub driver intentionally does not implement the full driver API.
 */
void platformRadioInit(void)
{
}

/**
 * platformRadioProcess — no-op.
 *
 * RT583 radio events (RX done, TX done) are delivered via lmac15p4 callbacks
 * registered in ot_radioInit().  Zephyr's radio.c version processes events via
 * the ieee802154 driver API which the RT583 stub does not implement.
 */
void platformRadioProcess(otInstance *aInstance)
{
    (void)aInstance;
}

/**
 * ot_uartTask — no-op stub for the Matter build.
 *
 * The OT UART CLI (ot_uart.c) is excluded in the Matter build because the OT
 * CLI sources are not compiled (OT_INCLUDE_CLI=0).  ot_zephyr.c's
 * otSysProcessDrivers() calls ot_uartTask(); provide a no-op so the linker
 * is satisfied.  The typedef below avoids including openthread_port.h which
 * has C++ incompatible inline functions.
 */
typedef uint32_t ot_system_event_t;
void ot_uartTask(ot_system_event_t sevent)
{
    (void)sevent;
}

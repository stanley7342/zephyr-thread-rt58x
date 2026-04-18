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
#include <zephyr/net/net_pkt.h>

/**
 * otSysInit — no-op.
 *
 * Zephyr's openthread.c (openthread_init) calls otSysInit() early in the OT L2
 * startup path.  The default implementation in modules/openthread/platform/
 * platform.c chains platformRadioInit / platformAlarm*Init — none of which
 * apply to RT583 (our radio is driven by ot_radioInit via lmac15p4 and our
 * alarm tick by ot_alarm.c).  We strip platform.c from the build and provide
 * this empty stub so the symbol still resolves at link time.
 */
void otSysInit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
}

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
 * notify_new_rx_frame / notify_new_tx_frame — no-op stubs.
 *
 * Zephyr's subsys/net/l2/openthread/openthread.c calls these when a net_pkt
 * traverses the L2 send / receive path.  In the default Zephyr build they live
 * in modules/openthread/platform/radio.c and push the packet into a FIFO for
 * the ieee802154 driver.  We strip radio.c (our ot_radio.c drives the RT583
 * lmac15p4 directly without net_pkt), so provide link-time stubs that return
 * failure — the L2 net_pkt path is unused in our radio architecture.
 */
int notify_new_rx_frame(struct net_pkt *pkt)
{
    (void)pkt;
    return -1;
}

int notify_new_tx_frame(struct net_pkt *pkt)
{
    (void)pkt;
    return -1;
}

/**
 * ot_uartTask — no-op stub for the Matter build only.
 *
 * When OT_INCLUDE_CLI=1 (Thread CLI example), ot_uart.c is compiled and
 * provides the real ot_uartTask implementation — this stub must be absent so
 * the real version wins the link.
 *
 * When OT_INCLUDE_CLI=0 (Matter build), ot_uart.c is NOT compiled; provide
 * this no-op so otSysProcessDrivers() (ot_zephyr.c) has a symbol to link
 * against.  The typedef avoids including openthread_port.h which has C++
 * incompatible inline functions.
 */
#if !defined(OT_INCLUDE_CLI) || !OT_INCLUDE_CLI
typedef uint32_t ot_system_event_t;
void ot_uartTask(ot_system_event_t sevent)
{
    (void)sevent;
}
#endif /* !OT_INCLUDE_CLI */

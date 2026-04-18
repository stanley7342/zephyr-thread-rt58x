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
#include <stddef.h>
#include <openthread/instance.h>
#include <openthread/error.h>
#include <openthread/platform/crypto.h>
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
#if defined(CONFIG_IEEE802154_RT583_FULL)
    /* Zephyr's modules/openthread/platform/radio.c owns the IEEE 802.15.4
     * driver handle (radio_dev / radio_api) and caches them in
     * platformRadioInit().  Since we strip platform.c from the build, we
     * must call platformRadioInit() ourselves here so radio_api gets set
     * before any radio_api-using code runs; otherwise the first call to
     * any otPlatRadio* NULL-derefs radio_api and reboots. */
    extern void platformRadioInit(void);
    platformRadioInit();
#endif
}

#if defined(CONFIG_IEEE802154_RT583_FULL)
/**
 * otPlatCryptoExportKey — stub for CONFIG_IEEE802154_RT583_FULL path.
 *
 * Zephyr's modules/openthread/platform/radio.c calls this from
 * otPlatRadioSetMacKey() when MAC keys are passed by reference.  The real
 * implementation lives in Zephyr's crypto_psa.c which we strip (duplicates
 * with ot_crypto.c otPlat* overrides).  Our OT config uses literal keys
 * (not key refs), so this path is never hit at runtime; provide a no-op
 * stub to satisfy the linker.
 */
otError otPlatCryptoExportKey(otCryptoKeyRef aKeyRef, uint8_t *aBuffer,
                              size_t aBufferLen, size_t *aKeyLen)
{
    (void)aKeyRef;
    (void)aBuffer;
    (void)aBufferLen;
    if (aKeyLen) {
        *aKeyLen = 0;
    }
    return OT_ERROR_NOT_IMPLEMENTED;
}

#else
/**
 * platformRadioInit / platformRadioProcess / notify_new_{rx,tx}_frame — no-op
 * stubs used only when CONFIG_IEEE802154_RT583_FULL=n.
 *
 * In the legacy path the real radio work is done by ot_radio.c directly via
 * lmac15p4 (bypassing Zephyr's ieee802154 driver / L2 net_pkt plumbing), so
 * Zephyr's modules/openthread/platform/radio.c is stripped out via
 * cmake/zephyr_module/CMakeLists.txt and these stubs satisfy remaining
 * link references from subsys/net/l2/openthread/openthread.c.
 *
 * When CONFIG_IEEE802154_RT583_FULL=y, radio.c is un-stripped and provides
 * the real implementations — these stubs must NOT be present to avoid
 * duplicate-definition errors.
 */
void platformRadioInit(void)
{
}

void platformRadioProcess(otInstance *aInstance)
{
    (void)aInstance;
}

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
#endif /* !CONFIG_IEEE802154_RT583_FULL */

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

/*
 * openthread-core-rt583-zephyr-config.h
 *
 * OpenThread compile-time configuration for RT583 on Zephyr.
 * Must be set as OPENTHREAD_CONFIG_FILE when building OT.
 */

#ifndef OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_
#define OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_

#include <openthread/logging.h>

/* Use otPlatSettings* API (SDK libopenthread-ftd.a calls these directly) */
/* OPENTHREAD_CONFIG_PLATFORM_FLASH_API_ENABLE is NOT set */

/* Route OT_ASSERT through otPlatAssertFail() so failures print file+line
 * instead of silently spinning in while(1). */
#define OPENTHREAD_CONFIG_PLATFORM_ASSERT_MANAGEMENT 1

/* Crypto backend: use OT's bundled mbedTLS (NOT Zephyr's PSA).
 *
 * Zephyr's crypto_psa.c (compiled from modules/openthread/platform/) checks
 * mContextSize >= sizeof(psa_mac_operation_t) from mbedTLS 4.0, but OT core
 * allocates sizeof(mbedtls_md_context_t) from the bundled mbedTLS 3.6.0.
 * These sizes differ (4.0 includes CMAC context in the union), so Zephyr's
 * checkContext() assertion fires.
 *
 * Fix: keep OPENTHREAD_CONFIG_CRYPTO_LIB_MBEDTLS (default).
 * OT's own crypto_platform.cpp (compiled from OT_CORE_SOURCES) provides all
 * otPlatCrypto* functions using OT's bundled mbedTLS — context sizes match.
 * With --allow-multiple-definition, crypto_platform.cpp (in libapp.a) takes
 * precedence over crypto_psa.c (in Zephyr's platform library).
 *
 * NOTE: OPENTHREAD_CONFIG_CRYPTO_LIB defaults to MBEDTLS(=0) so this line
 * is informational; the #ifndef guard in config/crypto.h handles the default. */
/* #define OPENTHREAD_CONFIG_CRYPTO_LIB OPENTHREAD_CONFIG_CRYPTO_LIB_MBEDTLS */

/* Microsecond alarm — TIMER3 hardware on RT583 */
#define OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE     1

/* FTD (Full Thread Device) */
#define OPENTHREAD_FTD                                   1
#define OPENTHREAD_CONFIG_THREAD_VERSION                 OT_THREAD_VERSION_1_3

/* CLI */
#define OPENTHREAD_CONFIG_CLI_MAX_LINE_LENGTH            256
#define OPENTHREAD_CONFIG_CLI_UART_TX_BUFFER_SIZE        512

/* Logging — set to NONE for production, DEBG for development */
#define OPENTHREAD_CONFIG_LOG_OUTPUT    OPENTHREAD_CONFIG_LOG_OUTPUT_APP
#define OPENTHREAD_CONFIG_LOG_LEVEL     OT_LOG_LEVEL_WARN

/* MAC */
#define OPENTHREAD_CONFIG_MAC_MAX_TX_ATTEMPTS_DIRECT             4
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_DIRECT   4

/* Heap — internal pool for OT dynamic allocations.
 * 6 KB covers: FTD idle state + BLE commissioning + SRP client registration
 * + SRP server service record storage (enabled for standalone Leader mode).
 * Raised from 4 KB to accommodate the SRP server storing the Matter
 * operational service record alongside the SRP client's working buffers. */
#define OPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE             (6 * 1024)

/* CoAP API — needed for OTA subsystem */
#define OPENTHREAD_CONFIG_COAP_API_ENABLE                1
#define OPENTHREAD_CONFIG_JOINER_ENABLE                  0

/* NCP disabled */
#define OPENTHREAD_CONFIG_NCP_HDLC_ENABLE                0

/* ECDSA — required by SRP client for signing */
#define OPENTHREAD_CONFIG_ECDSA_ENABLE                   1

/* SRP Client — required by Matter for service advertisement via Thread */
#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE              1
#define OPENTHREAD_CONFIG_SRP_CLIENT_AUTO_START_API_ENABLE 1

/* SRP Server — required when the RT583 is a standalone Thread Leader.
 * Without an OTBR, the device must run its own SRP server so the local
 * SRP client can register services and the Matter DNS-SD init callback
 * (OpenThreadDnssdInit) can fire.  AppTask enables/disables it via
 * otSrpServerSetEnabled() based on the Thread role.
 * Dependencies: TMF_NETDATA_SERVICE (explicit), NETDATA_PUBLISHER
 * (auto-derives from SRP_SERVER_ENABLE), ECDSA (already set above). */
#define OPENTHREAD_CONFIG_SRP_SERVER_ENABLE              1
#define OPENTHREAD_CONFIG_TMF_NETDATA_SERVICE_ENABLE     1

/* DNS Client — required by Matter for service discovery (mDNS-over-Thread) */
#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE              1
#define OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE 1

#endif /* OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_ */

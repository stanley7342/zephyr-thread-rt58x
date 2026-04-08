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

/* Heap — use platform malloc (newlib) */
#define OPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE             (8 * 1024)

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

/* DNS Client — required by Matter for service discovery (mDNS-over-Thread) */
#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE              1
#define OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE 1

#endif /* OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_ */

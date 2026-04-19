/*
 * openthread-core-rt583-zephyr-config.h
 *
 * OpenThread compile-time configuration for RT583 on Zephyr.
 * Aligned with Rafael official Matter SDK (OpenThreadConfig.h).
 * Must be set as OPENTHREAD_CONFIG_FILE when building OT.
 */

#ifndef OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_
#define OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_

/* ── Platform ──────────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_PLATFORM_INFO                  "RT58x"
#define OPENTHREAD_CONFIG_PLATFORM_ASSERT_MANAGEMENT     1
#define OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE     0
#define OPENTHREAD_CONFIG_PLATFORM_FLASH_API_ENABLE      0

/* ── Logging ───────────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_LOG_OUTPUT                      OPENTHREAD_CONFIG_LOG_OUTPUT_APP
#define OPENTHREAD_CONFIG_LOG_LEVEL                       OT_LOG_LEVEL_INFO
#define OPENTHREAD_CONFIG_LOG_LEVEL_DYNAMIC_ENABLE       0
#define OPENTHREAD_CONFIG_LOG_PLATFORM                   1

/* ── Heap ──────────────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE           1
#define OPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE             (2048 * sizeof(void *))

/* ── Radio / MAC ───────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_RADIO_2P4GHZ_OQPSK_SUPPORT    1
#define OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT    0
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_TX_SECURITY_ENABLE 1
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_CSMA_BACKOFF_ENABLE 0
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_RETRANSMIT_ENABLE 0
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_TX_TIMING_ENABLE  1
#define OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE        0
#define OPENTHREAD_CONFIG_MAC_CSL_REQUEST_AHEAD_US       0
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_DIRECT   4
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_INDIRECT 8
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS            80

/* ── MLE ───────────────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_MLE_MAX_CHILDREN               16
#define OPENTHREAD_CONFIG_MLE_ROUTER_SELECTION_JITTER    15
/* Leader Weight=1 so OTBR (weight=64) always wins partition merge */
#define OPENTHREAD_CONFIG_MLE_LEADER_WEIGHT              1

/* ── Crypto ────────────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_ECDSA_ENABLE                   1
#define OPENTHREAD_CONFIG_DETERMINISTIC_ECDSA_ENABLE     1
#define OPENTHREAD_CONFIG_ENABLE_BUILTIN_MBEDTLS_MANAGEMENT 0
#define OPENTHREAD_CONFIG_SECURE_TRANSPORT_ENABLE        0

/* ── Disable unused features (save flash) ──────────────────────────────── */

/* CoAP: disabled for Matter (saves flash), enabled for Thread CLI (OTA).
 * OT_INCLUDE_CLI is set to 0 for Matter, 1 for Thread CLI. */
#if OT_INCLUDE_CLI
#define OPENTHREAD_CONFIG_COAP_API_ENABLE                1
#else
#define OPENTHREAD_CONFIG_COAP_API_ENABLE                0
#endif
#define OPENTHREAD_CONFIG_JOINER_ENABLE                  0
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE            0
#define OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE             0
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE           0
#define OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE            0
#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE            0
#define OPENTHREAD_CONFIG_TCP_ENABLE                     0
#define OPENTHREAD_CONFIG_PING_SENDER_ENABLE             0
#define OPENTHREAD_CONFIG_NCP_HDLC_ENABLE                0
#define OPENTHREAD_CONFIG_DUA_ENABLE                     0

/* ── SRP Client (required by Matter for service advertisement) ─────────── */

#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE              1
#define OPENTHREAD_CONFIG_SRP_CLIENT_AUTO_START_API_ENABLE 1
/* Default 1800 ms is too short: OTBR reply can land after the RT583 has
 * just transitioned CHILD → ROUTER (rloc16 changes), plus OTBR SRP
 * server ECDSA SIG(0) verification + 6LoWPAN fragmentation take time.
 * Bump to 10 seconds so we wait long enough for the response. */
#define OPENTHREAD_CONFIG_SRP_CLIENT_MIN_RETRY_WAIT_INTERVAL 10000

/* ── SRP Server (standalone Leader mode — no OTBR) ─────────────────────── */

#define OPENTHREAD_CONFIG_SRP_SERVER_ENABLE              1
#define OPENTHREAD_CONFIG_TMF_NETDATA_SERVICE_ENABLE     1

/* ── DNS Client (required for CASE over Thread via OTBR) ───────────────── */

#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE              1
#define OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE 1

/* ── SLAAC (OMR address for SRP registration) ──────────────────────────── */

#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE               1

/* ── MLR (Thread 1.2+ multicast) ───────────────────────────────────────── */

#define OPENTHREAD_CONFIG_MLR_ENABLE                     1

/* ── CLI ───────────────────────────────────────────────────────────────── */

#define OPENTHREAD_CONFIG_CLI_UART_TX_BUFFER_SIZE        256

#endif /* OPENTHREAD_CORE_RT583_ZEPHYR_CONFIG_H_ */

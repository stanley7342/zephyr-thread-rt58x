/*
 * OTA subsystem — public API
 *
 * Flow:
 *   1. ota_coap_check()   — GET /ota/meta, compare version
 *   2. ota_coap_download()— GET /ota/image (Block2, 1024B/block) → staging
 *   3. ota_flash_apply()  — LZ4 decompress staging → slot1, SHA-256 verify
 *   4. sys_reboot()       — MCUboot picks slot1 if version is higher
 */

#ifndef OTA_H
#define OTA_H

#include <stdint.h>
#include <stdbool.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>

/* ── Meta packet (binary, server sends this at GET /ota/meta) ──────────── */
#define OTA_META_MAGIC  0x4F544100u   /* "OTA\0" */

typedef struct __attribute__((packed)) {
    uint32_t magic;             /* OTA_META_MAGIC                          */
    uint8_t  version[3];        /* [major, minor, patch]                   */
    uint8_t  _pad;
    uint32_t compressed_size;   /* bytes in staging after download         */
    uint32_t raw_size;          /* bytes in slot1 after decompress         */
    uint8_t  sha256_comp[32];   /* SHA-256 of compressed image             */
    uint8_t  sha256_raw[32];    /* SHA-256 of decompressed image           */
} ota_meta_t;                   /* total: 80 bytes                         */

/* ── OTA state (persisted in NVS) ─────────────────────────────────────── */
typedef enum {
    OTA_STATE_IDLE          = 0,
    OTA_STATE_DOWNLOADING   = 1,   /* writing to staging                   */
    OTA_STATE_DECOMPRESSING = 2,   /* staging → slot1                      */
    OTA_STATE_READY         = 3,   /* slot1 valid, waiting for reboot      */
    OTA_STATE_CONFIRMED     = 4,   /* new image running and confirmed OK   */
} ota_state_t;

/* ── Result codes ─────────────────────────────────────────────────────── */
#define OTA_OK              0
#define OTA_ERR_NO_UPDATE   1   /* current version is up to date          */
#define OTA_ERR_COAP        2   /* CoAP transport error                   */
#define OTA_ERR_HASH        3   /* SHA-256 mismatch                       */
#define OTA_ERR_DECOMP      4   /* LZ4 decompression error                */
#define OTA_ERR_FLASH       5   /* flash read/write error                 */
#define OTA_ERR_TIMEOUT     6   /* no response within timeout             */
#define OTA_ERR_BUSY        7   /* OTA already in progress                */

/* ── Initialise OTA subsystem ─────────────────────────────────────────── */
int  ota_init(otInstance *instance);

/* ── Check for new firmware (async, result via callback) ──────────────── */
typedef void (*ota_result_cb_t)(int result, const ota_meta_t *meta);
int  ota_check_and_download(const otIp6Address *server_addr,
                            uint16_t           server_port,
                            ota_result_cb_t    cb);

/* ── Apply downloaded image: decompress staging → slot1 ──────────────── */
/* Call after OTA_STATE_DOWNLOADING completes (can run in a low-prio thread) */
int  ota_flash_apply(const ota_meta_t *meta);

/* ── Confirm current image is healthy (call after successful boot) ─────── */
void ota_confirm(void);

/* ── Query current state ─────────────────────────────────────────────── */
ota_state_t ota_state_get(void);
uint32_t    ota_staging_written(void);

/* ── Resume interrupted download on next boot ─────────────────────────── */
int  ota_resume(const otIp6Address *server_addr, uint16_t server_port,
                ota_result_cb_t cb);

#endif /* OTA_H */

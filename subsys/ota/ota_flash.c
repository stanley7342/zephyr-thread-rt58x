/*
 * ota_flash.c — staging flash I/O + LZ4 decompress to slot1
 *
 * Staging layout (raw LZ4 block stream):
 *   [2B block_size][block_size B LZ4 data] repeated
 *   Each block decompresses to exactly CONFIG_OTA_LZ4_BLOCK_SZ bytes
 *   except the last block which may be smaller.
 *
 * RAM budget:
 *   lz4_in_buf   CONFIG_OTA_LZ4_BLOCK_SZ + LZ4_COMPRESSBOUND overhead  ~4.3 KB
 *   lz4_out_buf  CONFIG_OTA_LZ4_BLOCK_SZ                                 4.0 KB
 *   sha256 ctx   ~300 B
 *   total                                                                ~8.6 KB
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/settings/settings.h>
#include <string.h>

#include "ota.h"
#include "ota_internal.h"
#include "lz4.h"

/* mbedTLS SHA-256 — use OpenThread's bundled copy */
#include "mbedtls/sha256.h"

#define LOG_TAG  "[OTA-Flash] "

#define LZ4_BLOCK_SZ    CONFIG_OTA_LZ4_BLOCK_SZ          /* default 4096 */
#define LZ4_IN_MAXSZ    LZ4_COMPRESSBOUND(LZ4_BLOCK_SZ)  /* ~4114 bytes  */
#define SETTINGS_KEY    "ota/state"

/* ── Static buffers (BSS, not stack) ─────────────────────────────────── */
static uint8_t lz4_in_buf[LZ4_IN_MAXSZ];
static uint8_t lz4_out_buf[LZ4_BLOCK_SZ];

/* ── Persistent OTA state (NVS via Zephyr settings) ─────────────────── */
static struct {
    ota_state_t state;
    uint32_t    staging_written;   /* bytes written to staging so far      */
    uint8_t     version[3];        /* version being downloaded             */
} nvs_ctx;

static int settings_load_cb(const char *key, size_t len,
                            settings_read_cb read_cb, void *cb_arg)
{
    ARG_UNUSED(key);
    if (len == sizeof(nvs_ctx))
        read_cb(cb_arg, &nvs_ctx, sizeof(nvs_ctx));
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(ota, "ota", NULL,
                                settings_load_cb, NULL, NULL);

static void nvs_save(void)
{
    settings_save_one(SETTINGS_KEY, &nvs_ctx, sizeof(nvs_ctx));
}

/* ── Public state API ────────────────────────────────────────────────── */
void ota_state_set(ota_state_t state)
{
    nvs_ctx.state = state;
    nvs_save();
}

ota_state_t ota_state_get(void)
{
    return nvs_ctx.state;
}

uint32_t ota_staging_written(void)
{
    return nvs_ctx.staging_written;
}

bool ota_version_newer(const uint8_t ver[3])
{
    /* Compare server version against running image version from Kconfig */
    uint8_t cur[3] = {
        CONFIG_OTA_CURRENT_VERSION_MAJOR,
        CONFIG_OTA_CURRENT_VERSION_MINOR,
        CONFIG_OTA_CURRENT_VERSION_PATCH,
    };
    for (int i = 0; i < 3; i++) {
        if (ver[i] > cur[i]) return true;
        if (ver[i] < cur[i]) return false;
    }
    return false;   /* equal */
}

void ota_confirm(void)
{
    ota_state_set(OTA_STATE_CONFIRMED);
    printk(LOG_TAG "image confirmed\n");
}

/* ── Staging partition helpers ───────────────────────────────────────── */

/* Cached handle — opened for the lifetime of one download session */
static const struct flash_area *s_staging;

int ota_staging_open(void)
{
    if (s_staging) return 0;  /* already open */
    int rc = flash_area_open(PARTITION_ID(staging_partition), &s_staging);
    if (rc) { printk(LOG_TAG "staging open fail: %d\n", rc); s_staging = NULL; }
    return rc;
}

void ota_staging_close(void)
{
    if (s_staging) {
        flash_area_close(s_staging);
        s_staging = NULL;
    }
}

int ota_staging_erase(void)
{
    int rc = ota_staging_open();
    if (rc) return rc;

    printk(LOG_TAG "erasing staging (%zu KB)...\n", s_staging->fa_size / 1024);
    rc = flash_area_erase(s_staging, 0, s_staging->fa_size);

    nvs_ctx.staging_written = 0;
    nvs_save();

    if (rc) {
        printk(LOG_TAG "staging erase fail: %d\n", rc);
        ota_staging_close();
    } else {
        printk(LOG_TAG "staging erased\n");
    }
    return rc;
}

int ota_staging_write(uint32_t offset, const uint8_t *buf, uint16_t len)
{
    if (!s_staging) return -ENODEV;

    int rc = flash_area_write(s_staging, offset, buf, len);
    if (!rc) {
        nvs_ctx.staging_written = offset + len;
        nvs_save();
    }
    return rc;
}

/* ── Decompress staging → slot1 ─────────────────────────────────────── */
int ota_flash_apply(const ota_meta_t *meta)
{
    const struct flash_area *staging = NULL;
    const struct flash_area *slot1   = NULL;
    int rc = 0;

    printk(LOG_TAG "apply: decompress %u→%u bytes\n",
           meta->compressed_size, meta->raw_size);

    /* 1. Open partitions */
    rc = flash_area_open(PARTITION_ID(staging_partition), &staging);
    if (rc) { printk(LOG_TAG "open staging fail: %d\n", rc); goto out; }

    rc = flash_area_open(PARTITION_ID(slot1_partition), &slot1);
    if (rc) { printk(LOG_TAG "open slot1 fail: %d\n", rc); goto out; }

    /* 2. Verify SHA-256 of compressed data in staging */
    printk(LOG_TAG "verifying compressed SHA-256...\n");
    {
        mbedtls_sha256_context sha;
        mbedtls_sha256_init(&sha);
        mbedtls_sha256_starts(&sha, 0);

        uint32_t remaining = meta->compressed_size;
        uint32_t off = 0;
        while (remaining > 0) {
            uint16_t chunk = (remaining > sizeof(lz4_in_buf))
                             ? sizeof(lz4_in_buf) : (uint16_t)remaining;
            flash_area_read(staging, off, lz4_in_buf, chunk);
            mbedtls_sha256_update(&sha, lz4_in_buf, chunk);
            off += chunk;
            remaining -= chunk;
        }

        uint8_t actual[32];
        mbedtls_sha256_finish(&sha, actual);
        mbedtls_sha256_free(&sha);

        if (memcmp(actual, meta->sha256_comp, 32) != 0) {
            printk(LOG_TAG "compressed SHA-256 MISMATCH\n");
            rc = OTA_ERR_HASH; goto out;
        }
        printk(LOG_TAG "compressed SHA-256 OK\n");
    }

    /* 3. Erase slot1 */
    printk(LOG_TAG "erasing slot1 (%zu KB)...\n", slot1->fa_size / 1024);
    rc = flash_area_erase(slot1, 0, slot1->fa_size);
    if (rc) { printk(LOG_TAG "slot1 erase fail: %d\n", rc); goto out; }

    ota_state_set(OTA_STATE_DECOMPRESSING);

    /* 4. LZ4 block stream: [2B len][data]... → slot1 */
    {
        mbedtls_sha256_context sha;
        mbedtls_sha256_init(&sha);
        mbedtls_sha256_starts(&sha, 0);

        uint32_t src_off  = 0;   /* read position in staging */
        uint32_t dst_off  = 0;   /* write position in slot1  */
        uint32_t blocks   = 0;

        while (src_off < meta->compressed_size) {
            /* Read 2-byte block length prefix */
            uint16_t blk_len = 0;
            flash_area_read(staging, src_off, &blk_len, 2);
            src_off += 2;

            if (blk_len == 0 || blk_len > LZ4_IN_MAXSZ) {
                printk(LOG_TAG "bad block len %u at offset %u\n",
                       blk_len, src_off - 2);
                rc = OTA_ERR_DECOMP; goto sha_cleanup;
            }

            /* Read compressed block */
            flash_area_read(staging, src_off, lz4_in_buf, blk_len);
            src_off += blk_len;

            /* Decompress */
            int decoded = LZ4_decompress_safe(
                (const char *)lz4_in_buf,
                (char *)lz4_out_buf,
                blk_len, LZ4_BLOCK_SZ);

            if (decoded < 0) {
                printk(LOG_TAG "LZ4 error at block %u: %d\n",
                       blocks, decoded);
                rc = OTA_ERR_DECOMP; goto sha_cleanup;
            }

            /* Update raw SHA-256 */
            mbedtls_sha256_update(&sha, lz4_out_buf, decoded);

            /* RT583 flash write-block-size=1 — write exactly decoded bytes,
             * no padding needed. dst_off tracks actual image bytes. */
            rc = flash_area_write(slot1, dst_off, lz4_out_buf, (uint32_t)decoded);
            if (rc) {
                printk(LOG_TAG "slot1 write fail at %u: %d\n",
                       dst_off, rc);
                goto sha_cleanup;
            }
            dst_off += (uint32_t)decoded;
            blocks++;

            /* Progress: every 32 blocks (~128 KB) */
            if ((blocks & 0x1F) == 0) {
                printk(LOG_TAG "decomp %u/%u bytes\n",
                       dst_off, meta->raw_size);
            }
        }

        /* 5. Verify SHA-256 of decompressed image */
        printk(LOG_TAG "verifying raw SHA-256...\n");
        uint8_t actual[32];
        mbedtls_sha256_finish(&sha, actual);

        if (memcmp(actual, meta->sha256_raw, 32) != 0) {
            printk(LOG_TAG "raw SHA-256 MISMATCH — erasing slot1 header\n");
            flash_area_erase(slot1, 0, 4096);   /* invalidate MCUboot header */
            rc = OTA_ERR_HASH;
        } else {
            printk(LOG_TAG "raw SHA-256 OK  (%u bytes in %u blocks)\n",
                   dst_off, blocks);
            ota_state_set(OTA_STATE_READY);
            rc = OTA_OK;
        }

sha_cleanup:
        mbedtls_sha256_free(&sha);
    }

out:
    if (staging) flash_area_close(staging);
    if (slot1)   flash_area_close(slot1);
    return rc;
}

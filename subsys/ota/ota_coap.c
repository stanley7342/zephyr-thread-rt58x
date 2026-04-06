/*
 * ota_coap.c — CoAP Block-wise (RFC 7959) OTA download
 *
 * Protocol:
 *   Client (RT582)          Server (OTA host)
 *   ──────────────          ─────────────────
 *   GET /ota/meta    ──►    200  ota_meta_t (80B)
 *
 *   GET /ota/image   ──►    2.05 Block2 NUM=0  M=1  [1024B]
 *   GET /ota/image
 *     Block2 NUM=1   ──►    2.05 Block2 NUM=1  M=1  [1024B]
 *   ...
 *   GET /ota/image
 *     Block2 NUM=N   ──►    2.05 Block2 NUM=N  M=0  [<=1024B] (last)
 *
 * All requests are CON (confirmable). ACK timeout = 4 s, max retries = 3.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#include <openthread/coap.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/message.h>

#include "ota.h"
#include "ota_internal.h"

#define LOG_TAG         "[OTA-CoAP] "
#define COAP_PORT       5683u
#define BLOCK_SZX       OT_COAP_OPTION_BLOCK_SZX_1024   /* 1024 B/block  */
#define BLOCK_BYTES     1024u
#define REQ_TIMEOUT_MS  8000u    /* per-block timeout                     */
#define MAX_RETRIES     3u

/* ── Internal state ────────────────────────────────────────────────────── */
static struct {
    otInstance      *ot;
    otIp6Address     server_addr;
    uint16_t         server_port;
    ota_result_cb_t  cb;

    ota_meta_t       meta;
    bool             meta_valid;

    uint32_t         block_num;          /* next block to request          */
    uint32_t         bytes_written;      /* bytes written to staging so far*/
    uint8_t          retry;
    bool             resuming;           /* true: skip erase, start mid-dl */

    struct k_work_delayable timeout_work;
    bool             busy;
} s;

/* Forward declarations */
static void request_meta(void);
static void request_image_block(uint32_t num);
static void on_meta_response(void *ctx, otMessage *msg,
                             const otMessageInfo *info, otError err);
static void on_image_response(void *ctx, otMessage *msg,
                              const otMessageInfo *info, otError err);
static void timeout_handler(struct k_work *w);
static void finish(int result);

/* ── Init ──────────────────────────────────────────────────────────────── */
int ota_init(otInstance *instance)
{
    s.ot   = instance;
    s.busy = false;
    k_work_init_delayable(&s.timeout_work, timeout_handler);
    otCoapStart(instance, COAP_PORT);
    printk(LOG_TAG "init OK (CoAP port %u)\n", COAP_PORT);
    return OTA_OK;
}

/* ── Public: kick off check + download ────────────────────────────────── */
int ota_check_and_download(const otIp6Address *server_addr,
                           uint16_t            server_port,
                           ota_result_cb_t     cb)
{
    if (s.busy) return OTA_ERR_BUSY;

    s.server_addr   = *server_addr;
    s.server_port   = server_port ? server_port : COAP_PORT;
    s.cb            = cb;
    s.meta_valid    = false;
    s.block_num     = 0;
    s.bytes_written = 0;
    s.retry         = 0;
    s.resuming      = false;
    s.busy          = true;

    ota_state_set(OTA_STATE_IDLE);
    request_meta();
    return OTA_OK;
}

/* ── Resume interrupted download ─────────────────────────────────────── */
int ota_resume(const otIp6Address *server_addr, uint16_t server_port,
               ota_result_cb_t cb)
{
    if (s.busy) return OTA_ERR_BUSY;
    if (ota_state_get() != OTA_STATE_DOWNLOADING) return OTA_ERR_NO_UPDATE;

    uint32_t saved_offset = ota_staging_written();
    if (saved_offset == 0) return OTA_ERR_NO_UPDATE;

    s.server_addr   = *server_addr;
    s.server_port   = server_port ? server_port : COAP_PORT;
    s.cb            = cb;
    s.meta_valid    = false;
    s.bytes_written = saved_offset;
    s.block_num     = saved_offset / BLOCK_BYTES;
    s.retry         = 0;
    s.resuming      = true;   /* meta handler will skip erase            */
    s.busy          = true;

    /* Re-fetch meta to get fresh SHA-256 for final verification.
     * on_meta_response detects s.resuming and skips staging erase. */
    printk(LOG_TAG "resuming: re-fetching meta (saved offset %u, block %u)\n",
           s.bytes_written, s.block_num);
    request_meta();
    return OTA_OK;
}

/* ── Build CoAP message info for server ──────────────────────────────── */
static void fill_msg_info(otMessageInfo *info)
{
    memset(info, 0, sizeof(*info));
    info->mPeerAddr = s.server_addr;
    info->mPeerPort = s.server_port;
}

/* ── Step 1: GET /ota/meta ───────────────────────────────────────────── */
static void request_meta(void)
{
    otMessageInfo info;
    fill_msg_info(&info);

    otMessage *msg = otCoapNewMessage(s.ot, NULL);
    if (!msg) { finish(OTA_ERR_COAP); return; }

    otCoapMessageInit(msg, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);
    otCoapMessageAppendUriPathOptions(msg, "ota/meta");

    otError err = otCoapSendRequest(s.ot, msg, &info,
                                    on_meta_response, NULL);
    if (err != OT_ERROR_NONE) {
        printk(LOG_TAG "send meta request failed: %d\n", err);
        otMessageFree(msg);
        finish(OTA_ERR_COAP);
        return;
    }

    k_work_reschedule(&s.timeout_work, K_MSEC(REQ_TIMEOUT_MS));
    printk(LOG_TAG "GET /ota/meta sent\n");
}

/* ── Step 1 response handler ─────────────────────────────────────────── */
static void on_meta_response(void *ctx, otMessage *msg,
                             const otMessageInfo *info, otError err)
{
    ARG_UNUSED(ctx); ARG_UNUSED(info);
    k_work_cancel_delayable(&s.timeout_work);

    if (err != OT_ERROR_NONE || !msg) {
        printk(LOG_TAG "meta response error: %d\n", err);
        if (++s.retry <= MAX_RETRIES) { request_meta(); return; }
        finish(OTA_ERR_COAP); return;
    }

    otCoapCode code = otCoapMessageGetCode(msg);
    if (code != OT_COAP_CODE_CONTENT) {
        printk(LOG_TAG "meta: unexpected CoAP code 0x%02x\n", code);
        finish(OTA_ERR_COAP); return;
    }

    uint16_t len = otMessageGetLength(msg) - otMessageGetOffset(msg);
    if (len < sizeof(ota_meta_t)) {
        printk(LOG_TAG "meta: payload too short (%u)\n", len);
        finish(OTA_ERR_COAP); return;
    }

    otMessageRead(msg, otMessageGetOffset(msg), &s.meta, sizeof(ota_meta_t));

    if (s.meta.magic != OTA_META_MAGIC) {
        printk(LOG_TAG "meta: bad magic 0x%08x\n", s.meta.magic);
        finish(OTA_ERR_COAP); return;
    }

    printk(LOG_TAG "meta: v%u.%u.%u  comp=%u  raw=%u\n",
           s.meta.version[0], s.meta.version[1], s.meta.version[2],
           s.meta.compressed_size, s.meta.raw_size);

    /* Version check: only proceed if server version > running version */
    if (!ota_version_newer(s.meta.version)) {
        printk(LOG_TAG "already up to date\n");
        s.busy = false;
        if (s.cb) s.cb(OTA_ERR_NO_UPDATE, &s.meta);
        return;
    }

    s.meta_valid = true;
    s.retry      = 0;

    if (s.resuming) {
        /* Keep saved block_num / bytes_written; just open staging (no erase) */
        int rc = ota_staging_open();
        if (rc) { finish(OTA_ERR_FLASH); return; }
        printk(LOG_TAG "resuming from block %u (offset %u)\n",
               s.block_num, s.bytes_written);
    } else {
        s.block_num     = 0;
        s.bytes_written = 0;
        /* Erase staging; ota_staging_erase() leaves the partition open */
        int rc = ota_staging_erase();
        if (rc) { finish(OTA_ERR_FLASH); return; }
        ota_state_set(OTA_STATE_DOWNLOADING);
    }

    request_image_block(s.block_num);
}

/* ── Step 2: GET /ota/image  Block2 NUM=n ────────────────────────────── */
static void request_image_block(uint32_t num)
{
    otMessageInfo info;
    fill_msg_info(&info);

    otMessage *msg = otCoapNewMessage(s.ot, NULL);
    if (!msg) { finish(OTA_ERR_COAP); return; }

    otCoapMessageInit(msg, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);
    otCoapMessageAppendUriPathOptions(msg, "ota/image");
    otCoapMessageAppendBlock2Option(msg, num, false, BLOCK_SZX);

    otError err = otCoapSendRequest(s.ot, msg, &info,
                                    on_image_response, NULL);
    if (err != OT_ERROR_NONE) {
        printk(LOG_TAG "send block %u failed: %d\n", num, err);
        otMessageFree(msg);
        finish(OTA_ERR_COAP);
        return;
    }

    k_work_reschedule(&s.timeout_work, K_MSEC(REQ_TIMEOUT_MS));
}

/* ── Step 2 response handler (called per block) ──────────────────────── */
static uint8_t block_buf[BLOCK_BYTES];

static void on_image_response(void *ctx, otMessage *msg,
                              const otMessageInfo *info, otError err)
{
    ARG_UNUSED(ctx); ARG_UNUSED(info);
    k_work_cancel_delayable(&s.timeout_work);

    if (err != OT_ERROR_NONE || !msg) {
        printk(LOG_TAG "block %u error: %d (retry %u)\n",
               s.block_num, err, s.retry);
        if (++s.retry <= MAX_RETRIES) {
            request_image_block(s.block_num);
            return;
        }
        finish(OTA_ERR_COAP); return;
    }

    otCoapCode code = otCoapMessageGetCode(msg);
    if (code != OT_COAP_CODE_CONTENT) {
        printk(LOG_TAG "block %u: unexpected code 0x%02x\n",
               s.block_num, code);
        finish(OTA_ERR_COAP); return;
    }

    /* Parse Block2 option to get MORE flag.
     * If the option is missing or unreadable, treat as last block (more=false)
     * so the download completes rather than looping forever. */
    bool more = false;
    otCoapOptionIterator it;
    otCoapOptionIteratorInit(&it, msg);
    const otCoapOption *opt;
    while ((opt = otCoapOptionIteratorGetNextOption(&it)) != NULL) {
        if (opt->mNumber == OT_COAP_OPTION_BLOCK2) {
            uint64_t val = 0;
            if (otCoapOptionIteratorGetOptionUintValue(&it, &val) == OT_ERROR_NONE) {
                more = (bool)((val >> 3) & 1);   /* bit 3 = MORE flag */
            }
            break;
        }
    }

    /* Read block payload */
    uint16_t offset  = otMessageGetOffset(msg);
    uint16_t pay_len = otMessageGetLength(msg) - offset;
    if (pay_len > BLOCK_BYTES) pay_len = BLOCK_BYTES;

    otMessageRead(msg, offset, block_buf, pay_len);

    /* Write to staging flash */
    int rc = ota_staging_write(s.bytes_written, block_buf, pay_len);
    if (rc) { finish(OTA_ERR_FLASH); return; }

    s.bytes_written += pay_len;
    s.retry = 0;

    /* Progress log every 16 blocks (~16 KB) */
    if ((s.block_num & 0xF) == 0) {
        printk(LOG_TAG "block %u  %u/%u bytes\n",
               s.block_num, s.bytes_written, s.meta.compressed_size);
    }

    if (!more) {
        /* All blocks received */
        printk(LOG_TAG "download complete: %u bytes\n", s.bytes_written);
        finish(OTA_OK);
        return;
    }

    s.block_num++;
    request_image_block(s.block_num);
}

/* ── Timeout handler ─────────────────────────────────────────────────── */
static void timeout_handler(struct k_work *w)
{
    ARG_UNUSED(w);
    printk(LOG_TAG "timeout on block %u (retry %u)\n",
           s.block_num, s.retry);
    if (++s.retry <= MAX_RETRIES) {
        if (!s.meta_valid)  request_meta();
        else                request_image_block(s.block_num);
        return;
    }
    finish(OTA_ERR_TIMEOUT);
}

/* ── Finish: invoke callback, clear busy ─────────────────────────────── */
static void finish(int result)
{
    s.busy     = false;
    s.resuming = false;
    ota_staging_close();   /* no-op if already closed */
    if (result != OTA_OK) {
        ota_state_set(OTA_STATE_IDLE);
        printk(LOG_TAG "failed: %d\n", result);
    }
    if (s.cb) s.cb(result, s.meta_valid ? &s.meta : NULL);
}

/*
 * ot_crypto.c — OpenThread crypto platform for Zephyr on RT583
 *
 * All otPlatCrypto* functions are implemented here as NON-WEAK (strong)
 * symbols using OT's bundled mbedTLS 3.6.0.
 *
 * Why non-weak matters:
 *   crypto_platform.cpp (OT core) provides these as OT_TOOL_WEAK.
 *   Zephyr's crypto_psa.c (modules/openthread/platform/) provides them as
 *   STRONG symbols that check mContextSize >= sizeof(psa_mac_operation_t)
 *   from mbedTLS 4.0 — which is larger than what OT allocates from 3.6.0.
 *   Result: otPlatCryptoHmacSha256Init() assertion at hmac_sha256.cpp:47.
 *
 * Fix: provide STRONG implementations here (in libapp.a, linked first).
 *   --allow-multiple-definition: when both libapp.a and Zephyr's platform
 *   library define the same STRONG symbol, the first one wins (ours).
 *   These use sizeof(mbedtls_md_context_t) from OT's bundled mbedTLS 3.6.0
 *   which exactly matches what OT core allocates via context_size.hpp.
 *
 * Random: uses Rafael TRNG (hosal_trng_get_random_number) — no mbedTLS DRBG.
 */

#include <assert.h>
#include <string.h>

#include <openthread/config.h>
#include <openthread/platform/crypto.h>

/* OT bundled mbedTLS 3.6.0 headers — must come first in include order
 * (set via target_include_directories PRIVATE ${MBEDTLS_INC} in CMakeLists). */
#include <mbedtls/aes.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>

#include "openthread_port.h"
#include "hosal_trng.h"
#include <zephyr/sys/printk.h>

/* ── HMAC-SHA256 hash output size (32 bytes) ────────────────────────────── */
#define OT_HMAC_SHA256_SIZE 32

/* ── General init ───────────────────────────────────────────────────────── */

void otPlatCryptoInit(void)
{
    /* No-op: prevent Zephyr's PSA-based crypto init from running. */
}

/* ── Random (TRNG) ──────────────────────────────────────────────────────── */

void otPlatCryptoRandomInit(void)
{
    printk("[OT-CRYPTO] otPlatCryptoRandomInit\n");
    /* TRNG-based RNG needs no initialization beyond hosal_rf_init() */
}

void otPlatCryptoRandomDeinit(void)
{
}

otError otPlatCryptoRandomGet(uint8_t *aBuffer, uint16_t aSize)
{
    for (uint16_t i = 0; i < aSize; i++) {
        uint32_t r;
        hosal_trng_get_random_number(&r, 1);
        aBuffer[i] = (uint8_t)(r & 0xFF);
    }
    return OT_ERROR_NONE;
}

/* ── AES ────────────────────────────────────────────────────────────────── */

otError otPlatCryptoAesInit(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_aes_context)) return OT_ERROR_FAILED;
    mbedtls_aes_init((mbedtls_aes_context *)aContext->mContext);
    return OT_ERROR_NONE;
}

otError otPlatCryptoAesSetKey(otCryptoContext *aContext, const otCryptoKey *aKey)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_aes_context)) return OT_ERROR_FAILED;
    if (!aKey || !aKey->mKey) return OT_ERROR_INVALID_ARGS;
    int ret = mbedtls_aes_setkey_enc((mbedtls_aes_context *)aContext->mContext,
                                     aKey->mKey, (unsigned int)(aKey->mKeyLength * 8u));
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoAesEncrypt(otCryptoContext *aContext, const uint8_t *aInput, uint8_t *aOutput)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_aes_context)) return OT_ERROR_FAILED;
    int ret = mbedtls_aes_crypt_ecb((mbedtls_aes_context *)aContext->mContext,
                                    MBEDTLS_AES_ENCRYPT, aInput, aOutput);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoAesFree(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_aes_context)) return OT_ERROR_FAILED;
    mbedtls_aes_free((mbedtls_aes_context *)aContext->mContext);
    return OT_ERROR_NONE;
}

/* ── HMAC-SHA256 ────────────────────────────────────────────────────────── *
 * Context type: mbedtls_md_context_t (from OT's bundled mbedTLS 3.6.0).   *
 * Size matches kHmacSha256ContextSize in context_size.hpp.                */

otError otPlatCryptoHmacSha256Init(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_md_context_t)) return OT_ERROR_FAILED;

    mbedtls_md_context_t *ctx = (mbedtls_md_context_t *)aContext->mContext;
    mbedtls_md_init(ctx);
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    return (mbedtls_md_setup(ctx, info, 1 /* hmac */) == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoHmacSha256Deinit(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_md_context_t)) return OT_ERROR_FAILED;
    mbedtls_md_free((mbedtls_md_context_t *)aContext->mContext);
    return OT_ERROR_NONE;
}

otError otPlatCryptoHmacSha256Start(otCryptoContext *aContext, const otCryptoKey *aKey)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_md_context_t)) return OT_ERROR_FAILED;
    if (!aKey || !aKey->mKey) return OT_ERROR_INVALID_ARGS;

    int ret = mbedtls_md_hmac_starts((mbedtls_md_context_t *)aContext->mContext,
                                     aKey->mKey, aKey->mKeyLength);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoHmacSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_md_context_t)) return OT_ERROR_FAILED;

    int ret = mbedtls_md_hmac_update((mbedtls_md_context_t *)aContext->mContext,
                                     (const uint8_t *)aBuf, aBufLength);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoHmacSha256Finish(otCryptoContext *aContext, uint8_t *aBuf, size_t aBufLength)
{
    (void)aBufLength;
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_md_context_t)) return OT_ERROR_FAILED;

    int ret = mbedtls_md_hmac_finish((mbedtls_md_context_t *)aContext->mContext, aBuf);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

/* ── HKDF ───────────────────────────────────────────────────────────────── *
 * Context stores a 32-byte PRK (HMAC-SHA256 output).                       *
 * Mirrors kHkdfContextSize = sizeof(HmacSha256::Hash) = 32 bytes.         */

otError otPlatCryptoHkdfInit(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < OT_HMAC_SHA256_SIZE) return OT_ERROR_FAILED;
    memset(aContext->mContext, 0, OT_HMAC_SHA256_SIZE);
    return OT_ERROR_NONE;
}

otError otPlatCryptoHkdfDeinit(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < OT_HMAC_SHA256_SIZE) return OT_ERROR_FAILED;
    memset(aContext->mContext, 0, OT_HMAC_SHA256_SIZE);
    return OT_ERROR_NONE;
}

otError otPlatCryptoHkdfExtract(otCryptoContext   *aContext,
                                const uint8_t     *aSalt,
                                uint16_t           aSaltLength,
                                const otCryptoKey *aInputKey)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < OT_HMAC_SHA256_SIZE) return OT_ERROR_FAILED;
    if (!aInputKey || !aInputKey->mKey) return OT_ERROR_INVALID_ARGS;

    /* PRK = HMAC-SHA256(salt, IKM) */
    mbedtls_md_context_t md_ctx;
    const mbedtls_md_info_t *md_info;
    int ret;

    mbedtls_md_init(&md_ctx);
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    ret = mbedtls_md_setup(&md_ctx, md_info, 1 /* hmac */);
    if (ret != 0) goto exit;

    ret = mbedtls_md_hmac_starts(&md_ctx, aSalt, aSaltLength);
    if (ret != 0) goto exit;

    ret = mbedtls_md_hmac_update(&md_ctx, aInputKey->mKey, aInputKey->mKeyLength);
    if (ret != 0) goto exit;

    ret = mbedtls_md_hmac_finish(&md_ctx, (uint8_t *)aContext->mContext);

exit:
    mbedtls_md_free(&md_ctx);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoHkdfExpand(otCryptoContext *aContext,
                               const uint8_t   *aInfo,
                               uint16_t         aInfoLength,
                               uint8_t         *aOutputKey,
                               uint16_t         aOutputKeyLength)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < OT_HMAC_SHA256_SIZE) return OT_ERROR_FAILED;

    /* T(0) = empty; T(i) = HMAC-SHA256(PRK, T(i-1) || info || i) [RFC 5869] */
    uint8_t *prk  = (uint8_t *)aContext->mContext;
    uint8_t  hash[OT_HMAC_SHA256_SIZE];
    uint8_t  iter = 0;

    while (aOutputKeyLength > 0) {
        mbedtls_md_context_t md_ctx;
        const mbedtls_md_info_t *md_info;
        uint16_t copy_len;
        int ret;

        mbedtls_md_init(&md_ctx);
        md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        ret = mbedtls_md_setup(&md_ctx, md_info, 1);
        if (ret != 0) { mbedtls_md_free(&md_ctx); return OT_ERROR_FAILED; }

        ret = mbedtls_md_hmac_starts(&md_ctx, prk, OT_HMAC_SHA256_SIZE);
        if (ret != 0) { mbedtls_md_free(&md_ctx); return OT_ERROR_FAILED; }

        if (iter != 0) {
            ret = mbedtls_md_hmac_update(&md_ctx, hash, OT_HMAC_SHA256_SIZE);
            if (ret != 0) { mbedtls_md_free(&md_ctx); return OT_ERROR_FAILED; }
        }

        ret = mbedtls_md_hmac_update(&md_ctx, aInfo, aInfoLength);
        if (ret != 0) { mbedtls_md_free(&md_ctx); return OT_ERROR_FAILED; }

        iter++;
        ret = mbedtls_md_hmac_update(&md_ctx, &iter, 1);
        if (ret != 0) { mbedtls_md_free(&md_ctx); return OT_ERROR_FAILED; }

        ret = mbedtls_md_hmac_finish(&md_ctx, hash);
        mbedtls_md_free(&md_ctx);
        if (ret != 0) return OT_ERROR_FAILED;

        copy_len = (aOutputKeyLength < OT_HMAC_SHA256_SIZE)
                 ? aOutputKeyLength : OT_HMAC_SHA256_SIZE;
        memcpy(aOutputKey, hash, copy_len);
        aOutputKey      += copy_len;
        aOutputKeyLength -= copy_len;
    }

    return OT_ERROR_NONE;
}

/* ── SHA256 ─────────────────────────────────────────────────────────────── */

otError otPlatCryptoSha256Init(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    mbedtls_sha256_init((mbedtls_sha256_context *)aContext->mContext);
    return OT_ERROR_NONE;
}

otError otPlatCryptoSha256Deinit(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_sha256_context)) return OT_ERROR_FAILED;
    mbedtls_sha256_free((mbedtls_sha256_context *)aContext->mContext);
    aContext->mContext     = NULL;
    aContext->mContextSize = 0;
    return OT_ERROR_NONE;
}

otError otPlatCryptoSha256Start(otCryptoContext *aContext)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_sha256_context)) return OT_ERROR_FAILED;
    /* OT bundled mbedTLS >= 3.0.0: use mbedtls_sha256_starts (not _ret variant) */
    int ret = mbedtls_sha256_starts((mbedtls_sha256_context *)aContext->mContext, 0 /* is224=0 */);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength)
{
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_sha256_context)) return OT_ERROR_FAILED;
    int ret = mbedtls_sha256_update((mbedtls_sha256_context *)aContext->mContext,
                                    (const uint8_t *)aBuf, aBufLength);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatCryptoSha256Finish(otCryptoContext *aContext, uint8_t *aHash, uint16_t aHashSize)
{
    (void)aHashSize;
    if (!aContext || !aContext->mContext) return OT_ERROR_INVALID_ARGS;
    if (aContext->mContextSize < sizeof(mbedtls_sha256_context)) return OT_ERROR_FAILED;
    int ret = mbedtls_sha256_finish((mbedtls_sha256_context *)aContext->mContext, aHash);
    return (ret == 0) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}

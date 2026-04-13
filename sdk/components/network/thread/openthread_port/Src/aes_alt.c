/*
 * aes_alt.c — RT583 hardware AES backend for mbedTLS MBEDTLS_AES_ALT.
 *
 * Implements mbedtls_aes_init / free / setkey_enc / setkey_dec / crypt_ecb
 * using the RT583 AES hardware accelerator (rt_aes.h).
 *
 * AES and ECC share the same crypto accelerator.  Thread safety is provided by
 * aes_acquire() / aes_release() which call crypto_mutex_lock/unlock (implemented
 * in rt583_crypto_mutex.c using a Zephyr k_mutex).  CONFIG_SUPPORT_MULTITASKING
 * must be defined for those calls to be active.
 *
 * Every mbedtls_aes_crypt_ecb() call:
 *   1. aes_acquire()        — lock the shared accelerator
 *   2. aes_fw_init()        — (re)load AES firmware (needed if ECC ran last)
 *   3. aes_load_round_key() — (re)load the expanded round key registers
 *   4. aes_ecb_encrypt/decrypt() — hardware ECB operation
 *   5. aes_release()        — unlock the shared accelerator
 *
 * Only AES-128 is supported (MBEDTLS_AES_ONLY_128_BIT_KEY_LENGTH is defined).
 */

#include <string.h>
#include "mbedtls/aes.h"
#include "mbedtls/error.h"
#include "rt_aes.h"

void mbedtls_aes_init(mbedtls_aes_context *ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(*ctx));
    }
}

void mbedtls_aes_free(mbedtls_aes_context *ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(*ctx));  /* zero round key material */
    }
}

int mbedtls_aes_setkey_enc(mbedtls_aes_context *ctx,
                            const unsigned char *key,
                            unsigned int keybits)
{
    if (!ctx || !key) {
        return MBEDTLS_ERR_AES_INVALID_KEY_LENGTH;
    }
    if (keybits != 128) {
        /* Only AES-128 is supported by RT583 mbedTLS config */
        return MBEDTLS_ERR_AES_INVALID_KEY_LENGTH;
    }
    /* Software key expansion into hw_ctx.RoundKey — does NOT use hardware */
    aes_key_init(&ctx->hw_ctx, key, AES_KEY128);
    return 0;
}

int mbedtls_aes_setkey_dec(mbedtls_aes_context *ctx,
                            const unsigned char *key,
                            unsigned int keybits)
{
    /*
     * RT583 hardware uses the same expanded key for both encrypt and decrypt
     * (it applies inverse-MixColumns internally based on operation direction).
     * Re-use the encrypt key schedule.
     */
    return mbedtls_aes_setkey_enc(ctx, key, keybits);
}

int mbedtls_aes_crypt_ecb(mbedtls_aes_context *ctx,
                           int mode,
                           const unsigned char input[16],
                           unsigned char output[16])
{
    if (!ctx) {
        return MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH;
    }

    aes_acquire(&ctx->hw_ctx);          /* crypto_mutex_lock() */
    aes_fw_init();                      /* (re)load AES microcode */
    aes_load_round_key(&ctx->hw_ctx);   /* push round key registers */

    if (mode == MBEDTLS_AES_ENCRYPT) {
        aes_ecb_encrypt(&ctx->hw_ctx, (uint8_t *)input, output);
    } else {
        aes_ecb_decrypt(&ctx->hw_ctx, (uint8_t *)input, output);
    }

    aes_release(&ctx->hw_ctx);          /* crypto_mutex_unlock() */
    return 0;
}

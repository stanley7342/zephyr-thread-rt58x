#ifndef AES_ALT_H
#define AES_ALT_H

/*
 * RT583 hardware AES context for mbedTLS MBEDTLS_AES_ALT.
 *
 * Embeds struct aes_ctx directly so that the expanded round key (240 bytes)
 * is stored inline — no dynamic allocation needed. aes_key_init() performs
 * software key expansion into hw_ctx.RoundKey; subsequent mbedtls_aes_crypt_ecb()
 * calls lock the shared accelerator, reload AES firmware (in case ECC firmware
 * is currently loaded), load the round key, and run the hardware ECB engine.
 */

#include <stdint.h>
#include "rt_aes.h"   /* struct aes_ctx, aes_fw_init, aes_key_init, … */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct aes_ctx hw_ctx;  /* RT583 AES context — holds expanded round key (240 B) + IV (16 B) */
} mbedtls_aes_context;

typedef struct mbedtls_aes_xts_context {
    mbedtls_aes_context crypt;
    mbedtls_aes_context tweak;
} mbedtls_aes_xts_context;

#include "mbedtls/aes.h"

#ifdef __cplusplus
}
#endif

#endif /* AES_ALT_H */
/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

#include "mcu.h"

#include <string.h>

#include "rt_crypto.h"

#include "rt_ctr_drbg.h"


/*
 * Please Notice: this function block_cipher_df will need a "large" stack buffer!
 *
 */
static uint32_t block_cipher_df(uint8_t *output,
                                const uint8_t *data, uint32_t data_len )
{
    uint8_t buf[CTR_DRBG_MAX_SEED_INPUT + CTR_DRBG_BLOCKSIZE + 16];
    uint8_t tmp[CTR_DRBG_SEEDLEN];
    uint8_t key[CTR_DRBG_KEYSIZE];
    uint8_t chain[CTR_DRBG_BLOCKSIZE];
    uint8_t *p = buf, *iv;

    struct  aes_ctx  aes_cntx;

    int32_t i, j, buf_len, use_len;

    memset( buf, 0, CTR_DRBG_MAX_SEED_INPUT + CTR_DRBG_BLOCKSIZE + 16 );

    /*
     * Construct IV (16 bytes) and S in buffer
     * IV = Counter (in 32-bits) padded to 16 with zeroes
     * S = Length input string (in 32-bits) || Length of output (in 32-bits) ||
     *     data || 0x80
     *     (Total is padded to a multiple of 16-bytes with zeroes)
     */
    p = buf + CTR_DRBG_BLOCKSIZE;
    *p++ = ( data_len >> 24 ) & 0xff;
    *p++ = ( data_len >> 16 ) & 0xff;
    *p++ = ( data_len >> 8  ) & 0xff;
    *p++ = ( data_len       ) & 0xff;
    p += 3;
    *p++ = CTR_DRBG_SEEDLEN;
    memcpy( p, data, data_len );
    p[data_len] = 0x80;

    buf_len = CTR_DRBG_BLOCKSIZE + 8 + data_len + 1;

    for ( i = 0; i < CTR_DRBG_KEYSIZE; i++ )
    {
        key[i] = i;
    }

    /*set encryption key*/
    aes_key_init(&aes_cntx, key, CTR_DRBG_KEYSIZE_BIT);

    /*load key to secure engine*/
    aes_load_round_key(&aes_cntx);

    /*
     * Reduce data to CTR_DRBG_SEEDLEN bytes of data
     * For AES-128 it is 32 bytes, 2 blocks.
     * For AES-256 it is 48 bytes, 3 blocks.
     */
    for ( j = 0; j < CTR_DRBG_SEEDLEN; j += CTR_DRBG_BLOCKSIZE )
    {
        p = buf;
        memset( chain, 0, CTR_DRBG_BLOCKSIZE );
        use_len = buf_len;

        while ( use_len > 0 )
        {
            for ( i = 0; i < CTR_DRBG_BLOCKSIZE; i++ )
            {
                chain[i] ^= p[i];
            }
            p += CTR_DRBG_BLOCKSIZE;
            use_len -= CTR_DRBG_BLOCKSIZE;

            //aes_crypt_ecb( &aes_ctx, AES_ENCRYPT, chain, chain );
            aes_ecb_encrypt( &aes_cntx, chain, chain);
        }

        memcpy( tmp + j, chain, CTR_DRBG_BLOCKSIZE );

        /*
         * Update IV
         */
        buf[3]++;
    }

    /*
     * Do final encryption with reduced data
     */
    //aes_setkey_enc( &aes_cntx, tmp, CTR_DRBG_KEYBITS );
    aes_key_init(&aes_cntx, tmp, CTR_DRBG_KEYSIZE_BIT);

    /*load key to secure engine*/
    aes_load_round_key(&aes_cntx);

    iv = tmp + CTR_DRBG_KEYSIZE;
    p = output;

    for ( j = 0; j < CTR_DRBG_SEEDLEN; j += CTR_DRBG_BLOCKSIZE )
    {
        //aes_crypt_ecb( &aes_ctx, AES_ENCRYPT, iv, iv );
        aes_ecb_encrypt( &aes_cntx, iv, iv);
        memcpy( p, iv, CTR_DRBG_BLOCKSIZE );
        p += CTR_DRBG_BLOCKSIZE;
    }

    return STATUS_SUCCESS;
}

/* CTR_DRBG_Update (SP 800-90A &sect;10.2.1.2)
 * ctr_drbg_update_internal(ctx, provided_data)
 * implements
 * CTR_DRBG_Update(provided_data, Key, V)
 * with inputs and outputs
 *   ctx->aes_ctx = Key
 *   ctx->counter = V
 */
static uint32_t ctr_drbg_update_internal( ctr_drbg_context *ctx,
        const uint8_t data[CTR_DRBG_SEEDLEN])
{
    uint8_t  tmp[CTR_DRBG_SEEDLEN];
    uint8_t  *p = tmp;
    uint32_t i, j;

    memset( tmp, 0, CTR_DRBG_SEEDLEN );

    /*load key to secure engine*/
    aes_load_round_key(&(ctx->aes_cntx));

    for ( j = 0; j < CTR_DRBG_SEEDLEN; j += CTR_DRBG_BLOCKSIZE )
    {
        /*
         * Increase counter
         */
        for ( i = CTR_DRBG_BLOCKSIZE; i > 0; i-- )
            if ( ++ctx->counter[i - 1] != 0 )
            {
                break;
            }

        /*
         * Crypt counter block
         */
        aes_ecb_encrypt( &ctx->aes_cntx, ctx->counter, p);

        p += CTR_DRBG_BLOCKSIZE;
    }

    for ( i = 0; i < CTR_DRBG_SEEDLEN; i++ )
    {
        tmp[i] ^= data[i];
    }

    /*
     * Update key and counter
     */
    aes_key_init(&(ctx->aes_cntx), tmp, CTR_DRBG_KEYSIZE_BIT);

    memcpy( ctx->counter, tmp + CTR_DRBG_KEYSIZE, CTR_DRBG_BLOCKSIZE );

    memset(tmp, 0, CTR_DRBG_SEEDLEN);      /*destroy footprint*/

    return (STATUS_SUCCESS);
}


uint32_t ctr_drbg_reseed( ctr_drbg_context *ctx,
                          const uint8_t *additional, uint32_t len )
{
    uint8_t  seed[CTR_DRBG_MAX_SEED_INPUT];
    uint32_t seedlen = 0;

    if ( ctx->entropy_len + len > CTR_DRBG_MAX_SEED_INPUT )
    {
        return STATUS_INVALID_PARAM;
    }

    memset( seed, 0, CTR_DRBG_MAX_SEED_INPUT );

    /*
     * Gather enropy_len bytes of entropy to seed state
     */
    if ( 0 != ctx->f_entropy( ctx->p_entropy, seed,
                              ctx->entropy_len ) )
    {
        return STATUS_ERROR;
    }

    seedlen += ctx->entropy_len;

    /*
     * Add additional data
     */
    if ( additional && len )
    {
        memcpy( seed + seedlen, additional, len );
        seedlen += len;
    }

    /*
     * Reduce to 384 bits
     */
    block_cipher_df( seed, seed, seedlen );

    /*
     * Update state
     */
    ctr_drbg_update_internal( ctx, seed );
    ctx->reseed_counter = 1;

    return STATUS_SUCCESS;
}


/* CTR_DRBG_Instantiate with derivation function (SP 800-90A &sect;10.2.1.3.2)
 * ctr_drbg_init(ctx, f_entropy, p_entropy, custom, len)
 * implements
 * CTR_DRBG_Instantiate(entropy_input, nonce, personalization_string,
 *                      security_strength) -> initial_working_state
 * with inputs
 *   custom[:len] = nonce || personalization_string
 * where entropy_input comes from f_entropy for ctx->entropy_len bytes
 * and with outputs
 *   ctx = initial_working_state
 *
 * Notice: if custom len is zero, we will make entropy_length to be (3*CTR_DRBG_ENTROPY_LEN)/2
 *
 */

uint32_t ctr_drbg_init( ctr_drbg_context *ctx,
                        uint32_t (*f_entropy)(void *, uint8_t *, uint32_t),
                        void *p_entropy,
                        const uint8_t *custom,
                        uint32_t len)
{
    uint32_t ret;
    uint8_t  key[CTR_DRBG_KEYSIZE];

    memset( ctx, 0, sizeof(ctr_drbg_context) );
    memset( key, 0, CTR_DRBG_KEYSIZE );

    ctx->f_entropy = f_entropy;
    ctx->p_entropy = p_entropy;

    if (len == 0)
    {
        /* For AES-128, entropy length will become 256 bits */
        ctx->entropy_len = 3 * (CTR_DRBG_KEYSIZE) / 2;
    }
    else
    {
        ctx->entropy_len = CTR_DRBG_KEYSIZE;
    }

    ctx->reseed_interval = CTR_DRBG_RESEED_INTERVAL;

    /*
     * Initialize with an empty key
     */

    /*in fact, aes_acquire function is reserved for future multi-tasking case*/
    aes_acquire(&(ctx->aes_cntx));

    /*set encryption key*/
    aes_key_init(&(ctx->aes_cntx), key, CTR_DRBG_KEYSIZE_BIT);

    ret = ctr_drbg_reseed( ctx, custom, len );

    /*in fact, aes_release function is reserved for future multi-tasking case*/
    aes_release(&(ctx->aes_cntx));

    return ret;

}

void ctr_drbg_set_prediction_resistance( ctr_drbg_context *ctx,
        uint8_t resistance )
{
    ctx->prediction_resistance = resistance;
}


/* CTR_DRBG_Generate with derivation function (SP 800-90A &sect;10.2.1.5.2)
 * ctr_drbg_random_with_add(ctx, output, output_len, additional, add_len)
 * implements
 * CTR_DRBG_Reseed(working_state, entropy_input, additional[:add_len])
 *                -> working_state_after_reseed
 *                if required, then
 * CTR_DRBG_Generate(working_state_after_reseed,
 *                   requested_number_of_bits, additional_input)
 *                -> status, returned_bits, new_working_state
 * with inputs
 *   ctx contains working_state
 *   requested_number_of_bits = 8 * output_len
 *   additional[:add_len] = additional_input
 * and entropy_input comes from calling ctx->f_entropy
 * and with outputs
 *   status = SUCCESS (this function does the reseed internally)
 *   returned_bits = output[:output_len]
 *   ctx contains new_working_state
 */
uint32_t ctr_drbg_random_with_add(void *p_rng,
                                  uint8_t *output, uint32_t output_len,
                                  const uint8_t *additional, uint32_t add_len)
{
    int ret = STATUS_SUCCESS;

    ctr_drbg_context *ctx = (ctr_drbg_context *) p_rng;

    uint8_t  add_input[CTR_DRBG_SEEDLEN];
    uint8_t  *p = output;
    uint8_t  tmp[CTR_DRBG_BLOCKSIZE];

    int32_t  i;
    uint32_t use_len;

    if (output_len > CTR_DRBG_MAX_REQUEST)
    {
        return STATUS_INVALID_PARAM;        /*request random data too large */
    }

    if (add_len > CTR_DRBG_MAX_INPUT)
    {
        return STATUS_INVALID_PARAM;        /*input additional data too large */
    }

    memset(add_input, 0, CTR_DRBG_SEEDLEN);

    /*in fact, aes_acquire function is reserved for future multi-tasking case*/
    aes_acquire(&(ctx->aes_cntx));

    if (ctx->reseed_counter > ctx->reseed_interval ||
            ctx->prediction_resistance)
    {
        if ((ret = ctr_drbg_reseed(ctx, additional, add_len)) != STATUS_SUCCESS)
        {
            goto exit;
        }
        add_len = 0;
    }

    if (add_len > 0)
    {
        if ((ret = block_cipher_df(add_input, additional, add_len)) != STATUS_SUCCESS)
        {
            goto exit;
        }
        if ((ret = ctr_drbg_update_internal(ctx, add_input)) != STATUS_SUCCESS)
        {
            goto exit;
        }
    }

    /*load key to secure engine*/
    aes_load_round_key(&(ctx->aes_cntx));

    while (output_len > 0)
    {
        /*
         * Increase counter
         */
        for (i = CTR_DRBG_BLOCKSIZE; i > 0; i--)
        {
            if (++ctx->counter[i - 1] != 0)
            {
                break;
            }
        }

        /*
         * Crypt counter block
         */

        /*
         * Crypt counter block
         */
        aes_ecb_encrypt(&ctx->aes_cntx, ctx->counter, tmp);

        use_len = (output_len > CTR_DRBG_BLOCKSIZE)
                  ? CTR_DRBG_BLOCKSIZE : output_len;
        /*
         * Copy random block to destination
         */
        memcpy(p, tmp, use_len);
        p += use_len;
        output_len -= use_len;
    }

    if ((ret = ctr_drbg_update_internal(ctx, add_input)) != 0)
    {
        goto exit;
    }

    ctx->reseed_counter++;

exit:

    /*in fact, aes_release function is reserved for future multi-tasking case*/
    aes_release(&(ctx->aes_cntx));

    memset(add_input, 0, CTR_DRBG_SEEDLEN);
    memset(tmp, 0, CTR_DRBG_BLOCKSIZE);
    return ret;
}

uint32_t ctr_drbg_random(void *p_rng, uint8_t *output,
                         uint32_t output_len)
{
    return ctr_drbg_random_with_add(p_rng, output, output_len, NULL, 0);
}


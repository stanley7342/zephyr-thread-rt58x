/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**************************************************************************//**
 * @file     sha256.c
 * @version
 * @brief
 *
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "mcu.h"
#include "rt_crypto.h"
#define  INNER_PAD          0x36                    /*FIPS 198-1 Inner pad*/
#define  OUTER_PAD          0x5C                    /*FIPS 198-1 Inner pad*/

sha256_starts_rom_t    sha256_init = NULL;
sha256_update_rom_t    sha256_update = NULL;
sha256_finish_rom_t    sha256_finish = NULL;

void sha256_vector_init(void)
{
    /*global init once..*/
    sha256_init =  (sha256_starts_rom_t )  * ((uint32_t *)ROM_START_FUN_ADDR);
    sha256_update = (sha256_update_rom_t)   * ((uint32_t *)ROM_UPDATE_FUN_ADDR);
    sha256_finish = (sha256_finish_rom_t)   * ((uint32_t *)ROM_FINISH_FUN_ADDR);

}

static void rekey(uint8_t *ipad_key,  uint8_t *opad_key, const uint8_t *orig_key, unsigned int key_size)
{
    uint32_t i;

    for (i = 0; i < key_size; i++)
    {
        ipad_key[i] = INNER_PAD ^ orig_key[i];
        opad_key[i] = OUTER_PAD ^ orig_key[i];
    }

    for (; i < SHA256_BLOCK_SIZE; i++)
    {
        ipad_key[i] = INNER_PAD;
        opad_key[i] = OUTER_PAD;
    }
}


uint32_t hmac_sha256(const uint8_t *key,
                     uint32_t       key_length,
                     const uint8_t *msg,
                     uint32_t       msg_length,
                     uint8_t       *output)
{

    /*output buffer must be more than 32 bytes!!*/

    /*Please notice this function will need stack size over 200 bytes!*/
    sha256_context sha_cnxt;

    uint8_t   sha256_digest[SHA256_DIGEST_SIZE];

    /*ipad and opad, the first 64 bytes is ipad, the next 64 bytes is opad*/
    uint8_t   pad_key[SHA256_BLOCK_SIZE * 2];


    /*NOTICE: key_length can be zero!! But if key_length is not zero,,, then key can not BE NULL....*/
    if ((output == NULL) || ((key == NULL) && (key_length != 0)))
    {
        return STATUS_INVALID_PARAM  ;    /*not correct input*/
    }

    memset(pad_key, 0, sizeof(pad_key));

    /*
     * NOTICE: EVEN key is NULL pointer, and key_length is 0, it still can generate a HMAC  value
     * The value is 0x19EF24A32C717B167F33A91D6F648BDF96596776AFDB6377AC434C1C293CCB04
     */

    /*check key size*/


    /* if (key_length <= SHA256_BLOCK_SIZE)
     * The next three calls are dummy calls just to avoid
     * certain timing attacks. Without these dummy calls,
     * adversaries would be able to learn whether the key_length is
     * greater than SHA256_BLOCK_SIZE by measuring the time
     * consumed in this process.
     */
    sha256_init(&sha_cnxt);
    sha256_update(&sha_cnxt, (uint8_t *) key, key_length);
    sha256_finish(&sha_cnxt, sha256_digest);

    if (key_length <= SHA256_BLOCK_SIZE)
    {
        /* Actual code for when key_size <= SHA256_BLOCK_SIZE:
         * so only use the original key  and padding zero
         */
        rekey(pad_key, pad_key + SHA256_BLOCK_SIZE, key, key_length);
    }
    else
    {
        /* because input key is more than 64 bytes,
         * so sha256 the input key first to become new key
         */
        rekey(pad_key, pad_key + SHA256_BLOCK_SIZE, sha256_digest, SHA256_DIGEST_SIZE);
    }

    /*FIPS 198-1 step 5.   H( (K_0^ ipad)||text) */
    sha256_init(&sha_cnxt);
    sha256_update(&sha_cnxt, pad_key, SHA256_BLOCK_SIZE);
    sha256_update(&sha_cnxt, (uint8_t *)msg, msg_length);
    sha256_finish(&sha_cnxt, sha256_digest);

    /*FIPS 198-1 step 9.  H((K_0^opad)||( H( (K_0^ ipad)||text)) */

    sha256_init(&sha_cnxt);
    sha256_update(&sha_cnxt, (pad_key + SHA256_BLOCK_SIZE), SHA256_BLOCK_SIZE);
    sha256_update(&sha_cnxt, sha256_digest, SHA256_DIGEST_SIZE);
    sha256_finish(&sha_cnxt, output);

    /*clean data in stack...*/
    memset(pad_key, 0, (SHA256_BLOCK_SIZE * 2));
    memset(sha256_digest, 0, SHA256_DIGEST_SIZE);
    memset(&sha_cnxt, 0, sizeof (sha256_context));

    return STATUS_SUCCESS;
}

uint32_t hmac_sha256_init_ex(
    hmac_sha256_context  *cntx,
    const uint8_t   *key,
    uint32_t        key_length)
{
    uint8_t   sha256_digest[SHA256_DIGEST_SIZE];

    /*ipad and opad, the first 64 bytes is ipad, the next 64 bytes is opad*/
    uint8_t   ipad_key[SHA256_BLOCK_SIZE];

    /*
     * NOTICE: EVEN key is NULL pointer, and key_length is 0, it still can generate a HMAC  value
     * The value is 0x19EF24A32C717B167F33A91D6F648BDF96596776AFDB6377AC434C1C293CCB04
     */

    if (key_length <= SHA256_BLOCK_SIZE)
    {
        /* Actual code for when key_size <= SHA256_BLOCK_SIZE: */
        rekey(ipad_key, cntx->opad, key, key_length);
    }
    else
    {
        sha256_init((sha256_context *) cntx);
        sha256_update((sha256_context *) cntx, (uint8_t *) key, key_length);
        sha256_finish((sha256_context *) cntx, sha256_digest);

        rekey(ipad_key, cntx->opad, sha256_digest, SHA256_DIGEST_SIZE);
    }

    sha256_init((sha256_context *) cntx);
    sha256_update((sha256_context *) cntx, ipad_key, SHA256_BLOCK_SIZE);

    return STATUS_SUCCESS;
}

uint32_t hkdf_sha256(uint8_t *out_key, uint32_t out_len,
                     const uint8_t *secret, uint32_t secret_len,
                     const uint8_t *salt, uint32_t salt_len,
                     const uint8_t *info, uint32_t info_len)
{
    /* algortithm based https://tools.ietf.org/html/rfc5869#section-2.2 */

    uint32_t  n;
    uint32_t  i, tlen, copy_length;

    uint8_t   prk_digest[SHA256_DIGEST_SIZE];
    uint8_t   temp_digest[SHA256_BLOCK_SIZE];

    uint8_t   *ptr;

    hmac_sha256_context hkdf_sha_cntx;

    uint8_t   ctr = 1 ;

    /* according to RFC 5869, section 2.2 salt is optional...
     * if not provided, it is set to a string of HashLen zero!
     */

    n = (out_len + SHA256_DIGEST_SIZE - 1) >> 5;        /*because SHA256_DIGEST_SIZE is 32*/

    if ((out_key == NULL) || (n > 255))
    {
        return STATUS_INVALID_PARAM;
    }

    /*hmac_sha256 must return SHA256_DIGEST_SIZE bytes. */
    /*RFC5869 2.2. Step 1: Extract*/
    hmac_sha256(salt, salt_len, secret, secret_len, prk_digest);


    /*RFC5869 2.2. Step 2:  Expand*/

    tlen = 0;

    ptr = out_key;

    copy_length = out_len;

    for (i = 0; i < n; i++)
    {

        hmac_sha256_init_ex(&hkdf_sha_cntx, prk_digest, SHA256_DIGEST_SIZE);
        sha256_update((sha256_context *) &hkdf_sha_cntx, temp_digest, tlen);
        sha256_update((sha256_context *) &hkdf_sha_cntx, (uint8_t *) info, info_len);
        sha256_update((sha256_context *) &hkdf_sha_cntx, (uint8_t *) &ctr, 1);
        sha256_finish((sha256_context *) &hkdf_sha_cntx, temp_digest);

        /*caculate outpad*/
        sha256_init((sha256_context *)  &hkdf_sha_cntx);
        sha256_update((sha256_context *)  &hkdf_sha_cntx,  (hkdf_sha_cntx.opad), SHA256_BLOCK_SIZE);
        sha256_update((sha256_context *)  &hkdf_sha_cntx, temp_digest, SHA256_DIGEST_SIZE);
        sha256_finish((sha256_context *)  &hkdf_sha_cntx, temp_digest);

        if (copy_length > SHA256_DIGEST_SIZE)
        {
            /*copy SHA256_DIGEST_SIZE bytes data */
            memcpy((void *)ptr, temp_digest, SHA256_DIGEST_SIZE);
            ptr += SHA256_DIGEST_SIZE;
            copy_length -= SHA256_DIGEST_SIZE;
            tlen = SHA256_DIGEST_SIZE;

        }
        else
        {
            /*copy the last bytes*/
            memcpy((void *)ptr, temp_digest, copy_length);
            ptr += copy_length;
            copy_length = 0;
        }

        ctr++;

    }

    /*clear stack.*/
    ctr = 0;
    memset( (void *)prk_digest, 0, SHA256_DIGEST_SIZE);
    memset( (void *)temp_digest, 0, SHA256_DIGEST_SIZE);


    return STATUS_SUCCESS;
}


uint32_t pbkdf2_hmac(pbkdf2_st  *pb_vector)
{
    hmac_sha256_context   cntx_temp, cntx_temp_with_salt;
    hmac_sha256_context   pbkdf2_sha_cntx;

    uint32_t     i, j, counter = 1, gen_key_length = 0, counter_be, remain_length;
    uint8_t      temp_digest[SHA256_DIGEST_SIZE], mdl[SHA256_DIGEST_SIZE], work_buf[SHA256_DIGEST_SIZE];


    /*the password and salt, output key could not be NULL or zero length.*/
    if ((pb_vector->password == NULL) || (pb_vector->salt == NULL) || (pb_vector->key_output == NULL))
    {
        return STATUS_ERROR;
    }

    if ((pb_vector->password_length == 0) || (pb_vector->salt_length == 0) || (pb_vector->key_length == 0))
    {
        return STATUS_ERROR;
    }

    if (pb_vector->iteration > 100000)
    {
        return  STATUS_ERROR;   /*over matter maximum number iterations*/
    }

    /*caculate ipad, and opad first*/
    hmac_sha256_init_ex(&pbkdf2_sha_cntx, pb_vector->password, pb_vector->password_length);
    /*backup pbkdf2_sha_cntx without salt --- it will reused later.*/
    memcpy( &cntx_temp, &pbkdf2_sha_cntx, sizeof(hmac_sha256_context));

    sha256_update((sha256_context *)  &pbkdf2_sha_cntx, pb_vector->salt, pb_vector->salt_length);
    /*backup pbkdf2_sha_cntx --- it will reused later.*/
    memcpy( &cntx_temp_with_salt, &pbkdf2_sha_cntx, sizeof(hmac_sha256_context));

    remain_length = pb_vector->key_length - gen_key_length;

    while (gen_key_length < pb_vector->key_length)
    {
        counter_be = __REV(counter);    /*little endian to big endian*/
        sha256_update((sha256_context *)  &pbkdf2_sha_cntx, (uint8_t *) &counter_be, 4);

        sha256_finish((sha256_context *) &pbkdf2_sha_cntx, temp_digest);

        /*caculate outpad*/
        sha256_init((sha256_context *)  &pbkdf2_sha_cntx);
        sha256_update((sha256_context *)  &pbkdf2_sha_cntx,  (pbkdf2_sha_cntx.opad), SHA256_BLOCK_SIZE);

        sha256_update((sha256_context *)  &pbkdf2_sha_cntx, temp_digest, SHA256_DIGEST_SIZE);
        sha256_finish((sha256_context *)  &pbkdf2_sha_cntx, mdl);

        memcpy(work_buf, mdl, SHA256_DIGEST_SIZE);

        for (i = 1; i < pb_vector->iteration; i++)          /*Notice: i= 1*/
        {
            /*memcpy backup restore cntx_temp*/
            memcpy(&pbkdf2_sha_cntx, &cntx_temp, sizeof(hmac_sha256_context));
            sha256_update((sha256_context *)  &pbkdf2_sha_cntx, mdl, SHA256_DIGEST_SIZE);

            sha256_finish((sha256_context *) &pbkdf2_sha_cntx, temp_digest);

            /*caculate outpad*/
            sha256_init((sha256_context *)  &pbkdf2_sha_cntx);
            sha256_update((sha256_context *)  &pbkdf2_sha_cntx,  (pbkdf2_sha_cntx.opad), SHA256_BLOCK_SIZE);

            sha256_update((sha256_context *)  &pbkdf2_sha_cntx, temp_digest, SHA256_DIGEST_SIZE);
            sha256_finish((sha256_context *)  &pbkdf2_sha_cntx, mdl);

            for (j = 0; j < SHA256_DIGEST_SIZE; j++)
            {
                work_buf[j] ^= mdl[j];
            }

        }

        if (remain_length > SHA256_DIGEST_SIZE)
        {
            memcpy(pb_vector->key_output + gen_key_length, work_buf, SHA256_DIGEST_SIZE);
            gen_key_length += SHA256_DIGEST_SIZE;
            remain_length -= SHA256_DIGEST_SIZE;
            counter++;

            /*restore next pbkdf2_sha_cntx with salt*/
            memcpy( &pbkdf2_sha_cntx, &cntx_temp_with_salt, sizeof(hmac_sha256_context));
        }
        else
        {
            memcpy(pb_vector->key_output + gen_key_length, work_buf, remain_length);
            gen_key_length += remain_length;
            remain_length = 0;
            break;
        }

    }

    return STATUS_SUCCESS;
}





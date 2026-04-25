/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


#include <stdio.h>
#include <string.h>

#include "rt_crypto.h"


/*
 * drbg_update according NIST SP800-90A.
 * This DRBG is based HAMC. So SHA256_DIGEST_SIZE must be 0x20 bytes.
 *
 */
static void  hmac_drbg_update_index(hmac_drbg_state *state, uint8_t *data, uint32_t  data_length, uint32_t index)
{
    uint8_t   temp_index[4], tmp_msg[SHA256_BLOCK_SIZE];

    hmac_sha256_context   hmac_cntx;

#ifdef _DEBUG_
    uint32_t     i;
    uint8_t      *tmp_ptr;
#endif

    /* K= HMAC (K, V || "index" || provided_data).  index must be 0 or 1 */

    temp_index[0] = (uint8_t) (index & 0xFF);

    hmac_sha256_init_ex(&hmac_cntx, state->key, SHA256_DIGEST_SIZE);
    sha256_update((sha256_context *)&hmac_cntx, state->v, SHA256_DIGEST_SIZE);       /* V */
    sha256_update((sha256_context *)&hmac_cntx, temp_index, 1);             /* 0 */
    sha256_update((sha256_context *)&hmac_cntx, data, data_length);         /* provided_data */

    sha256_finish((sha256_context *)&hmac_cntx, tmp_msg);

    sha256_init((sha256_context *)  &hmac_cntx);
    sha256_starts((sha256_context *)  &hmac_cntx, 0);
    sha256_update((sha256_context *)  &hmac_cntx, hmac_cntx.opad, SHA256_BLOCK_SIZE);
    sha256_update((sha256_context *)  &hmac_cntx, tmp_msg, SHA256_DIGEST_SIZE);
    sha256_finish((sha256_context *)  &hmac_cntx, state->key);

#ifdef _DEBUG_
    printf("key is :");

    tmp_ptr = state->key;

    /*little_endian <-> big_endian is just  byte change, not include bit reverse.*/
    for (i = 0; i < SHA256_DIGEST_SIZE; i++)
    {
        printf("%02X ", tmp_ptr[i]);
    }

    printf("\n");
#endif

    /* K= HMAC (K, V) */

    hmac_sha256(state->key, SHA256_DIGEST_SIZE, state->v, SHA256_DIGEST_SIZE, state->v);

#ifdef _DEBUG_
    printf("v is :");

    tmp_ptr = state->v;

    /*little_endian <-> big_endian is just  byte change, not include bit reverse.*/
    for (i = 0; i < SHA256_DIGEST_SIZE; i++)
    {
        printf("%02X ", tmp_ptr[i]);
    }

    printf("\n \n");
#endif

}


void  hmac_drbg_update(hmac_drbg_state *state, uint8_t *data, uint32_t  data_length)
{
    /*according to SP800-90A page 44*/
    /*according to SP800-90A page 45, seed_material can be null string.*/
    hmac_drbg_update_index(state, data, data_length, 0);

    if (data != NULL)
    {
        hmac_drbg_update_index(state, data, data_length, 1);
    }

    return;

}


void  hmac_drbg_generate(uint8_t *out_result, hmac_drbg_state *state, uint32_t request_bytes,
                         uint8_t *data, uint32_t data_length)
{
    uint32_t    count_length;
    uint8_t     *copy_ptr;

    /*here we don't check out_result/state... it could not be NULL.*/

    if (data != NULL)
    {
        hmac_drbg_update(state, data, data_length);
    }

    count_length = 0;
    copy_ptr = out_result;

    while (count_length < request_bytes)
    {
        /*V = HMAC (Key, V)*/
        hmac_sha256(state->key, SHA256_DIGEST_SIZE, state->v, SHA256_DIGEST_SIZE, state->v);

        /* temp = temp||V */
        if ((request_bytes - count_length) > SHA256_DIGEST_SIZE)
        {
            memcpy(copy_ptr, state->v, SHA256_DIGEST_SIZE);
            copy_ptr += SHA256_DIGEST_SIZE;
            count_length += SHA256_DIGEST_SIZE;
        }
        else
        {
            memcpy(copy_ptr, state->v, (request_bytes - count_length));
            copy_ptr += request_bytes - count_length;
            break;      /*finish */
        }
    }

    /*NIST SP800-90A page 47 HMAC_DRBG Generate Process, step 6.*/

    hmac_drbg_update(state, data, data_length);

}

uint32_t hmac_drbg_instantiate(hmac_drbg_state *state, uint8_t *seed_material, uint32_t seed_material_length)
{
    /*according to SP800-90A 10.1.2.3 */

    memset(state->key, 0x0, SHA256_DIGEST_SIZE);

    memset(state->v, 0x01, SHA256_DIGEST_SIZE);

    hmac_drbg_update(state, seed_material, seed_material_length);

    return STATUS_SUCCESS;
}


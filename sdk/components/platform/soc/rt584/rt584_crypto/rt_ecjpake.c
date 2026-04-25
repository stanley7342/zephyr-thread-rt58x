/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**************************************************************************//**
 * @file     ecjpake.c
 * @version
 * @brief
 *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mcu.h"

#include "rt_crypto.h"


#if !defined(CONFIG_CRYPTO_SECP256R1_ENABLE)
#error "This project must set CRYPTO_SECP256R1_ENABLE option"
#endif


const  uint8_t  length_key[8] = {0, 0, 0, 0x41, 0x4};         /*in fact, 0x4 is not uncompressed format*/
const  uint8_t  length_id[4] = {0, 0, 0, 6};

const  uint8_t client_string[6] = {0x63, 0x6c, 0x69, 0x65, 0x6E, 0x74};
const  uint8_t server_string[6] = {0x73, 0x65, 0x72, 0x76, 0x65, 0x72};



/*
 * we force random number .. this is not true random number... just test
 * this function will be remove...
 *
 * Notice: this function will return big endian random number
 *
 *   maxim length is 128 bytes.
 *
 * This function will be remove later.... we should have a globe self test rng
 *
 */
static void self_test_rng(uint8_t *out, uint32_t length )
{
    static uint8_t entropy_input[32] =
    {
        0x05, 0x3c, 0x9f, 0xc4, 0xc6, 0x2a, 0x07, 0xe3, 0xf9, 0x08, 0x40, 0xda, 0x56, 0x16, 0x21, 0x8c,
        0x6d, 0xe5, 0x74, 0x3d, 0x66, 0xb8, 0xe0, 0xfb, 0xf8, 0x33, 0x75, 0x9c, 0x59, 0x28, 0xb5, 0x3d
    };

    static uint8_t nonce[16] =
    {
        0x2b, 0x89, 0xa1, 0x79, 0x04, 0x92, 0x2e, 0xd8, 0xf0, 0x17, 0xa6, 0x30, 0x44, 0x32, 0x85, 0x45
    };

    static uint8_t entropy_input_reseed[32] =
    {
        0x27, 0x91, 0x12, 0x6b, 0x8b, 0x44, 0xee, 0x1f, 0xd9, 0x39, 0x2a, 0x0a, 0x13, 0xe0, 0x08, 0x3b,
        0xed, 0x41, 0x82, 0xdc, 0x64, 0x9b, 0x73, 0x96, 0x07, 0xac, 0x70, 0xec, 0x8d, 0xce, 0xcf, 0x9b
    };

    static uint8_t additional_input_reseed[32] =
    {
        0x43, 0xba, 0xc1, 0x3b, 0xae, 0x71, 0x50, 0x92, 0x8f, 0x7e, 0xb2, 0x80, 0xa2, 0xe1, 0x1a, 0x96,
        0x2f, 0xaf, 0x72, 0x33, 0xc4, 0x14, 0x12, 0xf6, 0x9b, 0x32, 0x4a, 0x35, 0xa5, 0x84, 0xe5, 0x4c
    };

    static uint8_t additional_input[2][32] =
    {
        {
            0x3f, 0x2f, 0xed, 0x4b, 0x68, 0xd5, 0x06, 0xec, 0xef, 0xa2, 0x1f, 0x3f, 0x5b, 0xb9, 0x07, 0xbe,
            0xb0, 0xf1, 0x1d, 0xbc, 0x30, 0xf6, 0xf7, 0xbb, 0xa5, 0xe5, 0x86, 0x14, 0x08, 0xc5, 0x3a, 0x1e
        },
        {
            0x52, 0x90, 0x30, 0x4f, 0x50, 0x48, 0x10, 0x98, 0x5f, 0xde, 0x06, 0x8d, 0xf8, 0x2b, 0x93, 0x5e,
            0xc2, 0x3d, 0x83, 0x9c, 0xb4, 0xb2, 0x69, 0x41, 0x4c, 0x0e, 0xde, 0x6c, 0xff, 0xea, 0x5b, 0x68
        }
    };

    uint32_t *ptr,  *p_result;

    uint8_t  *pb_ptr;

    /*No PersonalizationString*/

    hmac_drbg_state    hmac_drbg_ctx;

    uint8_t seed_material[64];

    static uint8_t    result[128];      /*a buffer to save random bumber*/

    uint32_t     i;

    static uint8_t    *p_current = result;
    static uint32_t   remain_length = 0;

    if (remain_length >= length)
    {
        memcpy(out, p_current, length);
        p_current += length;
        remain_length -= length;
    }
    else
    {
        /*maybe we should use PRNG here*/

        /*regenerate new random number*/
        memcpy(seed_material, entropy_input, sizeof(entropy_input));
        memcpy(seed_material + sizeof(entropy_input), nonce, sizeof(nonce));

        hmac_drbg_instantiate(&hmac_drbg_ctx, seed_material, sizeof(entropy_input) + sizeof(nonce));

        /*drbg reseed  entropy_input+ additional_input_reseed*/

        memcpy(seed_material, entropy_input_reseed, sizeof(entropy_input_reseed));
        memcpy(seed_material + sizeof(entropy_input_reseed), additional_input_reseed, sizeof(additional_input_reseed));

        /*drbg reseed*/
        hmac_drbg_update(&hmac_drbg_ctx, (uint8_t *) seed_material, sizeof(entropy_input_reseed) + sizeof(additional_input_reseed));

        for (i = 0; i < 2; i++)
        {
            hmac_drbg_generate(result, &hmac_drbg_ctx, 128,  (uint8_t *)(additional_input[i]), 32);
        }

        p_result = (uint32_t *) result;

        /*maybe we should use PRNG HERE*/
        ptr = (uint32_t *) entropy_input;
        *ptr += *p_result++;

        pb_ptr = nonce;

        while (1)
        {

            *pb_ptr = *pb_ptr + 1;  /*increase one for next nonce*/

            if (*pb_ptr != 0)
            {
                break;      /*not overflow*/

            }

            pb_ptr++;
        }

        /*maybe we should use PRNG HERE*/
        ptr = (uint32_t *) entropy_input_reseed;
        *ptr += *p_result++;

        ptr = (uint32_t *) additional_input_reseed;
        *ptr += *p_result++;

        ptr = (uint32_t *) & (additional_input[0]);
        *ptr += *p_result++;

        ptr = (uint32_t *) & (additional_input[1]);
        *ptr += *p_result++;


        p_current = result;
        remain_length = 128;

        memcpy(out, p_current, length);
        p_current += length;
        remain_length -= length;

    }

    return;
}



#if 1

/*no debug message ... */

/*
 *
 *
 */

void ecjpake_generate_zkp(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key, ECPoint_P256 *gen, uint8_t *private_key)
{
    uint8_t   random_number_le[secp256r1_op_num_in_byte], private_key_le[secp256r1_op_num_in_byte];

    uint8_t   random_number[secp256r1_op_num_in_byte];

    ECPoint_P256   point, generator_le;

    sha256_context sha_cnxt;

    uint8_t        sha256_digest[SHA256_DIGEST_SIZE], sha256_digest_le[SHA256_DIGEST_SIZE] ;


    /*because private_key is big endian... we should convert it to little endian*/
    buffer_endian_exchange((uint32_t *)private_key_le, (uint32_t *) private_key, secp256r1_op_num);

    /*Please notice:  IMPORTANT sha256 can not use with ECC at the same time...*/

    /*transfer big endian generator to little endian*/
    buffer_endian_exchange((uint32_t *) (generator_le.x), (uint32_t *) gen->x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (generator_le.y), (uint32_t *) gen->y, secp256r1_op_num);

    gfp_ecc_curve_p256_init();

    /*
     * because operation is little endian.. but data format is big endian.
     * we need to convert big endian to little endian
     */
    gfp_point_p256_mult( (ECPoint_P256 *) &point,
                         (ECPoint_P256 *) &generator_le,
                         (uint32_t *) &private_key_le);


    /*transfer little endian to big endian.*/
    buffer_endian_exchange((uint32_t *) key->X.x, (uint32_t *) point.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) key->X.y, (uint32_t *) point.y, secp256r1_op_num);

    /*assume we should generate a random number... this is NOT TRNG... just for test.*/

    self_test_rng( random_number, secp256r1_op_num_in_byte);       /*random_number is big number*/


    /*convert random number to little endian*/
    buffer_endian_exchange((uint32_t *) random_number_le, (uint32_t *) random_number, secp256r1_op_num);

    gfp_ecc_curve_p256_init();

    gfp_point_p256_mult((ECPoint_P256 *) &point,
                        (ECPoint_P256 *) &generator_le,
                        (uint32_t *) random_number_le);


    buffer_endian_exchange((uint32_t *) key->zkp.V.x, (uint32_t *) point.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) key->zkp.V.y, (uint32_t *) point.y, secp256r1_op_num);


    sha256_init(&sha_cnxt);             /*init sha256 hash*/
    sha256_starts(&sha_cnxt, 0);
    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, (uint8_t *) gen->x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, (uint8_t *) gen->y, secp256r1_op_num_in_byte);

    /*ok now we have G, GV, and Key(X)  we should do sha256 for this message*/
    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->zkp.V.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->zkp.V.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt,  (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->X.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->X.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt, (uint8_t *) length_id, 4);

    if (ctx->role == server_role)
    {
        /*server*/
        sha256_update(&sha_cnxt, (uint8_t *) server_string, 6);
    }
    else
    {
        /*client*/
        sha256_update(&sha_cnxt, (uint8_t *) client_string, 6);
    }

    sha256_finish(&sha_cnxt, sha256_digest);

    /*Notice: buffer_endian_exchange is 4 bytes operation.. so count should be SHA256_DIGEST_SIZE/4 */

    buffer_endian_exchange((uint32_t *) sha256_digest_le,
                           (uint32_t *) sha256_digest, (SHA256_DIGEST_SIZE >> 2));

    /*now we need caculate random_number - x1*h   */

    /*now we need caculate b =  random_number - x*hash */

    /*temp use random_number.. because it is not used..*/
    gfp_scalar_vxh_p256((uint32_t *)random_number,  (uint32_t *) private_key_le,
                        (uint32_t *)sha256_digest_le, (uint32_t *) random_number_le);


    /*random_number is little endian... change it to big endian now.*/
    buffer_endian_exchange((uint32_t *) key->zkp.r,
                           (uint32_t *) random_number, secp256r1_op_num);

    /*clean memory footprint.  random_number already be destory..*/
    memset(random_number_le, 0, secp256r1_op_num_in_byte);

}


uint32_t ecjpake_verify_zkp(ECJPAKE_CTX  *ctx, ECPoint_P256 *generator, ECJPAKEKeyKP *key)
{
    ECPoint_P256   point_X, point_Gr, generator_le, point_Xh;

    sha256_context sha_cnxt;
    uint8_t        sha256_digest[SHA256_DIGEST_SIZE], sha256_digest_le[SHA256_DIGEST_SIZE] ;

    uint8_t        zkp_r_le[secp256r1_op_num_in_byte];         /*ZKP r*/
    uint32_t       status;

    /*first check  key->X is in curve.*/

    /*transfer big endian point X to little endian*/
    buffer_endian_exchange((uint32_t *) (point_X.x), (uint32_t *) key->X.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (point_X.y), (uint32_t *) key->X.y, secp256r1_op_num);

    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &point_X);

    if (status != STATUS_SUCCESS)
    {
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    buffer_endian_exchange((uint32_t *) (generator_le.x), (uint32_t *) generator->x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (generator_le.y), (uint32_t *) generator->y, secp256r1_op_num);

    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &generator_le);

    if (status != STATUS_SUCCESS)
    {
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*ok, we caculate sha256 first*/

    sha256_init(&sha_cnxt);             /*init sha256 hash*/
    sha256_starts(&sha_cnxt, 0);
    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, (uint8_t *) generator->x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, (uint8_t *) generator->y, secp256r1_op_num_in_byte);

    /*ok now we have G, GV, and Key(X)  we should do sha256 for this message*/
    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->zkp.V.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->zkp.V.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt,  (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->X.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->X.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt, (uint8_t *) length_id, 4);

    if (ctx->role == server_role)
    {
        /*we are server... so we check client*/
        sha256_update(&sha_cnxt, (uint8_t *) client_string, 6);
    }
    else
    {
        /*we are client... so we check server*/
        sha256_update(&sha_cnxt, (uint8_t *) server_string, 6);
    }

    sha256_finish(&sha_cnxt, sha256_digest);        /*ok we got sha256 digest big endian*/

    /*convert hash to little endian   */
    buffer_endian_exchange((uint32_t *) (sha256_digest_le), (uint32_t *) sha256_digest, (SHA256_DIGEST_SIZE >> 2));

    /*convert little endian zkp->r  */
    buffer_endian_exchange((uint32_t *) (zkp_r_le), (uint32_t *) key->zkp.r, secp256r1_op_num);

    /*caculate G*r point*/
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&point_Gr, (ECPoint_P256 *) &generator_le, (uint32_t *)  zkp_r_le);

    if (status != STATUS_SUCCESS)
    {
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&point_Xh, (ECPoint_P256 *) &point_X,  (uint32_t *)sha256_digest_le);

    if (status != STATUS_SUCCESS)
    {
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*temp resuse point_X to save space.*/
    status = gfp_point_p256_add((ECPoint_P256 *) &point_X,
                                (ECPoint_P256 *) &point_Gr, (ECPoint_P256 *)  &point_Xh);

    /*because point_X is little endian, so we should change it to big endian to compare */
    /*temp use buffer to save big endian data*/

    buffer_endian_exchange((uint32_t *) (point_Xh.x), (uint32_t *) point_X.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (point_Xh.y), (uint32_t *) point_X.y, secp256r1_op_num);

    /*compare caculate point and zkp point*/
    if ((memcmp(key->zkp.V.x, point_Xh.x, secp256r1_op_num) != 0) ||
            (memcmp(key->zkp.V.y, point_Xh.y, secp256r1_op_num) != 0))
    {
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}


uint32_t ecjpake_generate_step2_zkp(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key)
{
    ECPoint_P256  temp_point;
    ECPoint_P256  new_generator_le ;        /*point little endian*/

    uint8_t   secret_number_le[secp256r1_op_num_in_byte];
    uint8_t   secret_number[secp256r1_op_num_in_byte], secret_x2_le[secp256r1_op_num_in_byte];

    uint32_t  status;

    /*show debug message... remove later*/

    /*again we should convert point to little endian*/
    /*temp use key.X buffer... for saving memory*/
    buffer_endian_exchange((uint32_t *) & (key->X.x), (uint32_t *) (ctx->ecc_point_X1.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->X.y), (uint32_t *) (ctx->ecc_point_X1.y), secp256r1_op_num);


    /*temp use key.zkp.V for saving memory*/
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.x), (uint32_t *) (ctx->ecc_point_X3.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.y), (uint32_t *) (ctx->ecc_point_X3.y), secp256r1_op_num);

    /* temp_point = G*(x1+x3)*/
    gfp_point_p256_add( &(temp_point), &(key->X), &(key->zkp.V));

    /*temp use key.X buffer... for saving memory*/
    buffer_endian_exchange((uint32_t *) & (key->X.x), (uint32_t *) (ctx->ecc_point_X4.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->X.y), (uint32_t *) (ctx->ecc_point_X4.y), secp256r1_op_num);

    /* new_generator_le is G*(X1+X3+X4) .... little endian */
    gfp_point_p256_add( &(new_generator_le), &(temp_point), &(key->X));

    /*debug... verify the new generator is on curve??*/
    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &new_generator_le);

    if (status != STATUS_SUCCESS)
    {
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*convert generator to big endian*/
    /*temp use temp_point buffer... big endian now .. generator.. we need this convert because sha256 */
    buffer_endian_exchange((uint32_t *) & (temp_point.x), (uint32_t *) & (new_generator_le.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point.y), (uint32_t *) & (new_generator_le.y), secp256r1_op_num);

    /*generate xb*secret */

    /*convert "secret number" to little endian*/
    buffer_endian_exchange((uint32_t *) secret_number_le, (uint32_t *) ctx->secret, secp256r1_op_num);

    /*convert "private_key_x2 to little endian", temp use secret buffer*/
    buffer_endian_exchange((uint32_t *) secret_number, (uint32_t *) ctx->private_key_x2, secp256r1_op_num);

    /* secret_x2_le  = secret*private_x2 mod order */
    gfp_scalar_modmult_p256( (uint32_t *)secret_x2_le, (uint32_t *) secret_number_le,  (uint32_t *)secret_number);

    /*convert "secret*xb" to big endian*/
    buffer_endian_exchange((uint32_t *) secret_number, (uint32_t *) secret_x2_le, secp256r1_op_num);


    /*Notice: temp_point and secret_number is big endian... */
    ecjpake_generate_zkp(ctx, key, &temp_point, secret_number);

    return STATUS_SUCCESS;

}


uint32_t ecjpake_verify_step2_zkp(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key)
{
    ECPoint_P256  temp_point, temp_point2;
    ECPoint_P256  new_generator_le ;        /*point little endian*/

    uint32_t  status;

    /*again we should convert point to little endian*/
    /*because ECJPAKEKeyKP *key is waiting verify.. so we can not use it as temp memory here.*/
    buffer_endian_exchange((uint32_t *) & (temp_point2.x), (uint32_t *) (ctx->ecc_point_X1.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point2.y), (uint32_t *) (ctx->ecc_point_X1.y), secp256r1_op_num);

    /*temp use new_generator_le for little endian transfer*/
    buffer_endian_exchange((uint32_t *) & (new_generator_le.x), (uint32_t *) (ctx->ecc_point_X2.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (new_generator_le.y), (uint32_t *) (ctx->ecc_point_X2.y), secp256r1_op_num);

    /* temp_point = G*(x1+x3)*/
    gfp_point_p256_add( &(temp_point), &(temp_point2), &(new_generator_le));

    /*temp use temp_point2 buffer... for saving memory*/
    buffer_endian_exchange((uint32_t *) & (temp_point2.x), (uint32_t *) (ctx->ecc_point_X3.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point2.y), (uint32_t *) (ctx->ecc_point_X3.y), secp256r1_op_num);

    /* new_generator_le is G*(X1+X3+X4) .... little endian */
    gfp_point_p256_add( &(new_generator_le), &(temp_point), &(temp_point2));

    /*debug... verify the new generator is on curve??*/
    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &new_generator_le);

    if (status != STATUS_SUCCESS)
    {

        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*convert generator to big endian*/
    /*temp use temp_point buffer... big endian now .. generator.. we need this convert because sha256 */
    buffer_endian_exchange((uint32_t *) & (temp_point.x), (uint32_t *) & (new_generator_le.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point.y), (uint32_t *) & (new_generator_le.y), secp256r1_op_num);

    status = ecjpake_verify_zkp(ctx, &(temp_point), key);

    if (status != STATUS_SUCCESS)
    {
        while (1);
    }

    return STATUS_SUCCESS;
}


uint32_t ecjpake_compute_key(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key, uint8_t *pms)
{
    ECPoint_P256   point, temp_point;

    uint8_t   key_le[secp256r1_op_num_in_byte];

    uint32_t  status;

    sha256_context sha_cnxt;

    /*in fact, we should check length...but we don't do it*/

    if ((ctx == NULL) || (key == NULL) || (pms == NULL))
    {
        return STATUS_ERROR;
    }

    /*transfer peer public key X4 to little endian   */
    /*temp use key->zkp.V  because it is not used anymore...*/
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.x), (uint32_t *) (ctx->ecc_point_X4.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.y), (uint32_t *) (ctx->ecc_point_X4.y), secp256r1_op_num);

    /*transfer local private key2 to little endian*/
    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) (ctx->private_key_x2), secp256r1_op_num);

    /* caculate G*x2*x4 */
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&point, (ECPoint_P256 *) &key->zkp.V, (uint32_t *)  key_le);

    if (status != STATUS_SUCCESS)
    {
        /*errror.... Notice: key_le could not more than order...*/
        return status;
    }

    /*Notice: point is little endian..*/
    gfp_point_p256_invert(&point, &point);

    /*transfer share secret to little endian*/
    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) (ctx->secret), secp256r1_op_num);

    /*temp use key->zkp.V  because it is not used anymore...*/
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&(key->zkp.V), (ECPoint_P256 *) &point, (uint32_t *) key_le);

    if (status != STATUS_SUCCESS)
    {
        /*errror.... Notice: key_le could not more than order...*/
        return status;
    }

    /*key->zkp.V is little endian*/
    status = gfp_valid_point_p256_verify(&(key->zkp.V));

    if (status !=  STATUS_SUCCESS)
    {
        return status;      /*why?? almost impossible*/
    }


    /*transfer Step2 Gx to little endian */
    buffer_endian_exchange((uint32_t *) & (temp_point.x), (uint32_t *) & (key->X.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point.y), (uint32_t *) & (key->X.y), secp256r1_op_num);

    status = gfp_point_p256_add(&point, &(temp_point), &(key->zkp.V));

    /*transfer private_key2 to little endian*/

    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) (ctx->private_key_x2), secp256r1_op_num);

    /*Notice: key->zkp.V is little endian*/
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&(key->zkp.V), (ECPoint_P256 *) &point, (uint32_t *) key_le);

    if (status != STATUS_SUCCESS)
    {
        /*errror.... Notice: key_le could not more than order...*/
        return status;
    }

    /*caculate pms*/
    /*transfer PMSK X coordinate from little endian to big endian*/
    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) & (key->zkp.V.x), secp256r1_op_num);


    sha256_init(&sha_cnxt);             /*init sha256 hash*/
    sha256_starts(&sha_cnxt, 0); 
    sha256_update(&sha_cnxt, (uint8_t *) key_le, secp256r1_op_num_in_byte);

    sha256_finish(&sha_cnxt, pms);

    /*TODO: destroy PMSK information and key_le */

    return STATUS_SUCCESS;

}

#else



/*
 *
 *
 */

void ecjpake_generate_zkp(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key, ECPoint_P256 *gen, uint8_t *private_key)
{
    uint8_t   random_number_le[secp256r1_op_num_in_byte], private_key_le[secp256r1_op_num_in_byte];

    uint8_t   random_number[secp256r1_op_num_in_byte];

    ECPoint_P256   point, generator_le;

    sha256_context sha_cnxt;

    uint8_t        sha256_digest[SHA256_DIGEST_SIZE], sha256_digest_le[SHA256_DIGEST_SIZE] ;


    /*because private_key is big endian... we should convert it to little endian*/
    buffer_endian_exchange((uint32_t *)private_key_le, (uint32_t *) private_key, secp256r1_op_num);

    /*Please notice:  IMPORTANT sha256 can not use with ECC at the same time...*/

    /*transfer big endian generator to little endian*/
    buffer_endian_exchange((uint32_t *) (generator_le.x), (uint32_t *) gen->x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (generator_le.y), (uint32_t *) gen->y, secp256r1_op_num);

    gfp_ecc_curve_p256_init();

    /*
     * because operation is little endian.. but data format is big endian.
     * we need to convert big endian to little endian
     */
    gfp_point_p256_mult( (ECPoint_P256 *) &point,
                         (ECPoint_P256 *) &generator_le,
                         (uint32_t *) &private_key_le);

    printf("ECPoint X: \n");

    becp_print_hex_bytes(point.x, secp256r1_op_num_in_byte);
    printf("\n");

    becp_print_hex_bytes(point.y, secp256r1_op_num_in_byte);
    printf("\n");

    /*transfer little endian to big endian.*/
    buffer_endian_exchange((uint32_t *) key->X.x, (uint32_t *) point.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) key->X.y, (uint32_t *) point.y, secp256r1_op_num);

    /*assume we should generate a random number... this is NOT TRNG... just for test.*/

    self_test_rng( random_number, secp256r1_op_num_in_byte);       /*random_number is big number*/

    printf("random number (big endian): \n");

    lecp_print_hex_bytes( random_number, secp256r1_op_num_in_byte);     /* 1 2 3 4 .... native order display*/

    printf("\n");

    /*convert random number to little endian*/
    buffer_endian_exchange((uint32_t *) random_number_le, (uint32_t *) random_number, secp256r1_op_num);

    gfp_ecc_curve_p256_init();

    gfp_point_p256_mult((ECPoint_P256 *) &point,
                        (ECPoint_P256 *) &generator_le,
                        (uint32_t *) random_number_le);

    printf("ECPoint V: \n");

    becp_print_hex_bytes(point.x, secp256r1_op_num_in_byte);
    printf("\n");

    becp_print_hex_bytes(point.y, secp256r1_op_num_in_byte);
    printf("\n");

    buffer_endian_exchange((uint32_t *) key->zkp.V.x, (uint32_t *) point.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) key->zkp.V.y, (uint32_t *) point.y, secp256r1_op_num);

    printf("\n");

    printf("sha256 result:");

    sha256_init(&sha_cnxt);             /*init sha256 hash*/

    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, (uint8_t *) gen->x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, (uint8_t *) gen->y, secp256r1_op_num_in_byte);

    /*ok now we have G, GV, and Key(X)  we should do sha256 for this message*/
    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->zkp.V.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->zkp.V.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt,  (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->X.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->X.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt, (uint8_t *) length_id, 4);

    if (ctx->role == server_role)
    {
        /*server*/
        sha256_update(&sha_cnxt, (uint8_t *) server_string, 6);
    }
    else
    {
        /*client*/
        sha256_update(&sha_cnxt, (uint8_t *) client_string, 6);
    }


    sha256_finish(&sha_cnxt, sha256_digest);

    printf("sha256 result  (big endian): \n");

    lecp_print_hex_bytes( sha256_digest, SHA256_DIGEST_SIZE);

    printf("\n");

    /*Notice: buffer_endian_exchange is 4 bytes operation.. so count should be SHA256_DIGEST_SIZE/4 */

    buffer_endian_exchange((uint32_t *) sha256_digest_le,
                           (uint32_t *) sha256_digest, (SHA256_DIGEST_SIZE >> 2));

    /*now we need caculate random_number - x1*h   */

    /*now we need caculate b =  random_number - x*hash */

    /*temp use random_number.. because it is not used..*/
    gfp_scalar_vxh_p256((uint32_t *)random_number,  (uint32_t *) private_key_le,
                        (uint32_t *)sha256_digest_le, (uint32_t *) random_number_le);


    /*random_number is little endian... change it to big endian now.*/
    buffer_endian_exchange((uint32_t *) key->zkp.r,
                           (uint32_t *) random_number, secp256r1_op_num);

    printf("zkp b number  (big endian): \n");

    lecp_print_hex_bytes( key->zkp.r, secp256r1_op_num_in_byte);

    /*clean memory footprint.  random_number already be destory..*/
    memset(random_number_le, 0, secp256r1_op_num_in_byte);

    printf("**********\n\n");


}


uint32_t ecjpake_verify_zkp(ECJPAKE_CTX  *ctx, ECPoint_P256 *generator, ECJPAKEKeyKP *key)
{
    ECPoint_P256   point_X, point_Gr, generator_le, point_Xh;

    sha256_context sha_cnxt;
    uint8_t        sha256_digest[SHA256_DIGEST_SIZE], sha256_digest_le[SHA256_DIGEST_SIZE] ;

    uint8_t        zkp_r_le[secp256r1_op_num_in_byte];         /*ZKP r*/
    uint32_t       status;

    printf("generator (big endian) : \n");

    lecp_print_hex_bytes(generator->x, secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes(generator->y, secp256r1_op_num_in_byte);
    printf("\n");


    printf("key X1 big endian : \n");

    lecp_print_hex_bytes(key->X.x, secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes(key->X.y, secp256r1_op_num_in_byte);
    printf("\n");

    printf("zkp V big endian : \n");

    lecp_print_hex_bytes(key->zkp.V.x, secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes(key->zkp.V.y, secp256r1_op_num_in_byte);
    printf("\n");

    printf("zkp big number (big endian) : \n");

    lecp_print_hex_bytes(key->zkp.r, secp256r1_op_num_in_byte);
    printf("\n");

    /*first check  key->X is in curve.*/

    /*transfer big endian point X to little endian*/
    buffer_endian_exchange((uint32_t *) (point_X.x), (uint32_t *) key->X.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (point_X.y), (uint32_t *) key->X.y, secp256r1_op_num);

    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &point_X);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... Key X is not valid point \n");
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    buffer_endian_exchange((uint32_t *) (generator_le.x), (uint32_t *) generator->x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (generator_le.y), (uint32_t *) generator->y, secp256r1_op_num);

    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &generator_le);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... generator is not valid point \n");
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*ok, we caculate sha256 first*/

    sha256_init(&sha_cnxt);             /*init sha256 hash*/

    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, (uint8_t *) generator->x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, (uint8_t *) generator->y, secp256r1_op_num_in_byte);

    /*ok now we have G, GV, and Key(X)  we should do sha256 for this message*/
    sha256_update(&sha_cnxt, (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->zkp.V.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->zkp.V.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt,  (uint8_t *) length_key, 5);

    sha256_update(&sha_cnxt, key->X.x, secp256r1_op_num_in_byte);
    sha256_update(&sha_cnxt, key->X.y, secp256r1_op_num_in_byte);

    sha256_update(&sha_cnxt, (uint8_t *) length_id, 4);

    if (ctx->role == server_role)
    {
        /*we are server... so we check client*/
        sha256_update(&sha_cnxt, (uint8_t *) client_string, 6);
    }
    else
    {
        /*we are client... so we check server*/
        sha256_update(&sha_cnxt, (uint8_t *) server_string, 6);
    }

    sha256_finish(&sha_cnxt, sha256_digest);        /*ok we got sha256 digest big endian*/


    lecp_print_hex_bytes(sha256_digest, SHA256_DIGEST_SIZE);
    printf("\n");

    /*convert hash to little endian   */
    buffer_endian_exchange((uint32_t *) (sha256_digest_le), (uint32_t *) sha256_digest, (SHA256_DIGEST_SIZE >> 2));

    /*convert little endian zkp->r  */
    buffer_endian_exchange((uint32_t *) (zkp_r_le), (uint32_t *) key->zkp.r, secp256r1_op_num);

    /*caculate G*r point*/
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&point_Gr, (ECPoint_P256 *) &generator_le, (uint32_t *)  zkp_r_le);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... G*r error?? \n");
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&point_Xh, (ECPoint_P256 *) &point_X,  (uint32_t *)sha256_digest_le);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... G*r error?? \n");
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*temp resuse point_X to save space.*/
    status = gfp_point_p256_add((ECPoint_P256 *) &point_X,
                                (ECPoint_P256 *) &point_Gr, (ECPoint_P256 *)  &point_Xh);

    printf("result point: \n");

    becp_print_hex_bytes(point_X.x, secp256r1_op_num_in_byte);
    printf("\n");

    becp_print_hex_bytes(point_X.y, secp256r1_op_num_in_byte);
    printf("\n");

    /*because point_X is little endian, so we should change it to big endian to compare */
    /*temp use buffer to save big endian data*/

    buffer_endian_exchange((uint32_t *) (point_Xh.x), (uint32_t *) point_X.x, secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) (point_Xh.y), (uint32_t *) point_X.y, secp256r1_op_num);

    /*compare caculate point and zkp point*/
    if ((memcmp(key->zkp.V.x, point_Xh.x, secp256r1_op_num) != 0) ||
            (memcmp(key->zkp.V.y, point_Xh.y, secp256r1_op_num) != 0))
    {
        return STATUS_ERROR;
    }

    printf("zkp verify success \n");

    return STATUS_SUCCESS;
}


uint32_t ecjpake_generate_step2_zkp(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key)
{
    ECPoint_P256  temp_point;
    ECPoint_P256  new_generator_le ;        /*point little endian*/

    uint8_t   secret_number_le[secp256r1_op_num_in_byte];
    uint8_t   secret_number[secp256r1_op_num_in_byte], secret_x2_le[secp256r1_op_num_in_byte];

    uint32_t  status;


    printf("ecjpake_generate_step2_zkp \n");


    /*show debug message... remove later*/

    printf("ctx->ecc_point_X1 point (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) (ctx->ecc_point_X1.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) (ctx->ecc_point_X1.y), secp256r1_op_num_in_byte);
    printf("\n");

    printf("ctx->ecc_point_X3 point (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) (ctx->ecc_point_X3.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) (ctx->ecc_point_X3.y), secp256r1_op_num_in_byte);
    printf("\n");


    printf("ctx->ecc_point_X4 point (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) (ctx->ecc_point_X4.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) (ctx->ecc_point_X4.y), secp256r1_op_num_in_byte);
    printf("\n");


    /*again we should convert point to little endian*/
    /*temp use key.X buffer... for saving memory*/
    buffer_endian_exchange((uint32_t *) & (key->X.x), (uint32_t *) (ctx->ecc_point_X1.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->X.y), (uint32_t *) (ctx->ecc_point_X1.y), secp256r1_op_num);


    /*temp use key.zkp.V for saving memory*/
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.x), (uint32_t *) (ctx->ecc_point_X3.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.y), (uint32_t *) (ctx->ecc_point_X3.y), secp256r1_op_num);

    /* temp_point = G*(x1+x3)*/
    gfp_point_p256_add( &(temp_point), &(key->X), &(key->zkp.V));

    /*temp use key.X buffer... for saving memory*/
    buffer_endian_exchange((uint32_t *) & (key->X.x), (uint32_t *) (ctx->ecc_point_X4.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->X.y), (uint32_t *) (ctx->ecc_point_X4.y), secp256r1_op_num);

    /* new_generator_le is G*(X1+X3+X4) .... little endian */
    gfp_point_p256_add( &(new_generator_le), &(temp_point), &(key->X));

    /*debug... verify the new generator is on curve??*/
    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &new_generator_le);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... new generator new_generator_le is not valid point \n");
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }


    /*convert generator to big endian*/
    /*temp use temp_point buffer... big endian now .. generator.. we need this convert because sha256 */
    buffer_endian_exchange((uint32_t *) & (temp_point.x), (uint32_t *) & (new_generator_le.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point.y), (uint32_t *) & (new_generator_le.y), secp256r1_op_num);


    printf("new G' point: (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) & (temp_point.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) & (temp_point.y), secp256r1_op_num_in_byte);
    printf("\n");


    /*generate xb*secret */

    /*convert "secret number" to little endian*/
    buffer_endian_exchange((uint32_t *) secret_number_le, (uint32_t *) ctx->secret, secp256r1_op_num);

    printf("secret_number_le:  \n");
    lecp_print_hex_bytes( (uint8_t *) secret_number_le, secp256r1_op_num_in_byte);
    printf("\n");


    /*convert "private_key_x2 to little endian", temp use secret buffer*/
    buffer_endian_exchange((uint32_t *) secret_number, (uint32_t *) ctx->private_key_x2, secp256r1_op_num);


    printf("private key:   \n");
    lecp_print_hex_bytes( (uint8_t *) secret_number, secp256r1_op_num_in_byte);
    printf("\n");

    /* secret_x2_le  = secret*private_x2 mod order */
    gfp_scalar_modmult_p256( (uint32_t *)secret_x2_le, (uint32_t *) secret_number_le,  (uint32_t *)secret_number);

    /*convert "secret*xb" to big endian*/
    buffer_endian_exchange((uint32_t *) secret_number, (uint32_t *) secret_x2_le, secp256r1_op_num);


    printf("secret xbs \n");
    lecp_print_hex_bytes( (uint8_t *) secret_number, secp256r1_op_num_in_byte);
    printf("\n");

    /*Notice: temp_point and secret_number is big endian... */
    ecjpake_generate_zkp(ctx, key, &temp_point, secret_number);

    return STATUS_SUCCESS;

}


uint32_t ecjpake_verify_step2_zkp(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key)
{
    ECPoint_P256  temp_point, temp_point2;
    ECPoint_P256  new_generator_le ;        /*point little endian*/

    uint32_t  status;

    printf("ecjpake_verify_step2_zkp \n");

    /*show debug message... remove later*/

    printf("ctx->ecc_point_X1 point (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) (ctx->ecc_point_X1.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) (ctx->ecc_point_X1.y), secp256r1_op_num_in_byte);
    printf("\n");

    printf("ctx->ecc_point_X2 point (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) (ctx->ecc_point_X2.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) (ctx->ecc_point_X2.y), secp256r1_op_num_in_byte);
    printf("\n");

    printf("ctx->ecc_point_X3 point (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) (ctx->ecc_point_X3.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) (ctx->ecc_point_X3.y), secp256r1_op_num_in_byte);
    printf("\n");

    /*again we should convert point to little endian*/
    /*because ECJPAKEKeyKP *key is waiting verify.. so we can not use it as temp memory here.*/
    buffer_endian_exchange((uint32_t *) & (temp_point2.x), (uint32_t *) (ctx->ecc_point_X1.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point2.y), (uint32_t *) (ctx->ecc_point_X1.y), secp256r1_op_num);

    /*temp use new_generator_le for little endian transfer*/
    buffer_endian_exchange((uint32_t *) & (new_generator_le.x), (uint32_t *) (ctx->ecc_point_X2.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (new_generator_le.y), (uint32_t *) (ctx->ecc_point_X2.y), secp256r1_op_num);

    /* temp_point = G*(x1+x3)*/
    gfp_point_p256_add( &(temp_point), &(temp_point2), &(new_generator_le));

    /*temp use temp_point2 buffer... for saving memory*/
    buffer_endian_exchange((uint32_t *) & (temp_point2.x), (uint32_t *) (ctx->ecc_point_X3.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point2.y), (uint32_t *) (ctx->ecc_point_X3.y), secp256r1_op_num);

    /* new_generator_le is G*(X1+X3+X4) .... little endian */
    gfp_point_p256_add( &(new_generator_le), &(temp_point), &(temp_point2));

    /*debug... verify the new generator is on curve??*/
    status =  gfp_valid_point_p256_verify((ECPoint_P256 *) &new_generator_le);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... new generator new_generator_le is not valid point \n");
        return STATUS_ERROR;                /* ECJPAKEKeyKP.X point is not on curve... */
    }

    /*convert generator to big endian*/
    /*temp use temp_point buffer... big endian now .. generator.. we need this convert because sha256 */
    buffer_endian_exchange((uint32_t *) & (temp_point.x), (uint32_t *) & (new_generator_le.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point.y), (uint32_t *) & (new_generator_le.y), secp256r1_op_num);

    printf("new G' point: (big endian): \n");

    lecp_print_hex_bytes( (uint8_t *) & (temp_point.x), secp256r1_op_num_in_byte);
    printf("\n");

    lecp_print_hex_bytes((uint8_t *) & (temp_point.y), secp256r1_op_num_in_byte);
    printf("\n");

    status = ecjpake_verify_zkp(ctx, &(temp_point), key);

    if (status != STATUS_SUCCESS)
    {
        printf("Oops... ZKP step2 verify error \n");
        while (1);
    }

    printf("ZKP step2 verify  correct \n");
    return STATUS_SUCCESS;
}


uint32_t ecjpake_compute_key(ECJPAKE_CTX  *ctx, ECJPAKEKeyKP *key, uint8_t *pms)
{
    ECPoint_P256   point, temp_point;

    uint8_t   key_le[secp256r1_op_num_in_byte];

    uint32_t  status;

    sha256_context sha_cnxt;

    /*in fact, we should check length...but we don't do it*/

    if ((ctx == NULL) || (key == NULL) || (pms == NULL))
    {
        return STATUS_ERROR;
    }

    /*transfer peer public key X4 to little endian   */
    /*temp use key->zkp.V  because it is not used anymore...*/
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.x), (uint32_t *) (ctx->ecc_point_X4.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (key->zkp.V.y), (uint32_t *) (ctx->ecc_point_X4.y), secp256r1_op_num);

    /*transfer local private key2 to little endian*/
    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) (ctx->private_key_x2), secp256r1_op_num);

    /* caculate G*x2*x4 */
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&point, (ECPoint_P256 *) &key->zkp.V, (uint32_t *)  key_le);

    if (status != STATUS_SUCCESS)
    {
        /*errror.... Notice: key_le could not more than order...*/
        return status;
    }

    /*caculate -G*x4*x2 */
    printf("G*x2*x4 point: (big endian order): \n");

    becp_print_hex_bytes( (uint8_t *) & (point.x), secp256r1_op_num_in_byte);
    printf("\n");

    becp_print_hex_bytes((uint8_t *) & (point.y), secp256r1_op_num_in_byte);
    printf("\n");


    /*Notice: point is little endian..*/
    gfp_point_p256_invert(&point, &point);

    printf("invert G*x2*x4 point: (big endian order): \n");

    becp_print_hex_bytes( (uint8_t *) & (point.x), secp256r1_op_num_in_byte);
    printf("\n");

    becp_print_hex_bytes((uint8_t *) & (point.y), secp256r1_op_num_in_byte);
    printf("\n");

    /*transfer share secret to little endian*/
    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) (ctx->secret), secp256r1_op_num);

    /*temp use key->zkp.V  because it is not used anymore...*/
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&(key->zkp.V), (ECPoint_P256 *) &point, (uint32_t *) key_le);

    if (status != STATUS_SUCCESS)
    {
        /*errror.... Notice: key_le could not more than order...*/
        return status;
    }

    /*key->zkp.V is little endian*/
    status = gfp_valid_point_p256_verify(&(key->zkp.V));

    if (status !=  STATUS_SUCCESS)
    {
        return status;      /*why?? almost impossible*/
    }


    /*transfer Step2 Gx to little endian */
    buffer_endian_exchange((uint32_t *) & (temp_point.x), (uint32_t *) & (key->X.x), secp256r1_op_num);
    buffer_endian_exchange((uint32_t *) & (temp_point.y), (uint32_t *) & (key->X.y), secp256r1_op_num);

    status = gfp_point_p256_add(&point, &(temp_point), &(key->zkp.V));

    /*transfer private_key2 to little endian*/

    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) (ctx->private_key_x2), secp256r1_op_num);

    /*Notice: key->zkp.V is little endian*/
    gfp_ecc_curve_p256_init();
    status = gfp_point_p256_mult(&(key->zkp.V), (ECPoint_P256 *) &point, (uint32_t *) key_le);

    if (status != STATUS_SUCCESS)
    {
        /*errror.... Notice: key_le could not more than order...*/
        return status;
    }

    /*show k message*/
    printf("PMSK point: (big endian order): \n");

    becp_print_hex_bytes( (uint8_t *) & (key->zkp.V.x), secp256r1_op_num_in_byte);
    printf("\n");

    becp_print_hex_bytes((uint8_t *) & (key->zkp.V.y), secp256r1_op_num_in_byte);
    printf("\n");

    /*caculate pms*/
    /*transfer PMSK X coordinate from little endian to big endian*/
    buffer_endian_exchange((uint32_t *) & (key_le), (uint32_t *) & (key->zkp.V.x), secp256r1_op_num);


    sha256_init(&sha_cnxt);             /*init sha256 hash*/

    sha256_update(&sha_cnxt, (uint8_t *) key_le, secp256r1_op_num_in_byte);

    sha256_finish(&sha_cnxt, pms);


    printf("PMS: (big endian order): \n");

    lecp_print_hex_bytes( (uint8_t *) pms, secp256r1_op_num_in_byte);
    printf("\n");

    /*TODO: destroy PMSK information and key_le */

    return STATUS_SUCCESS;

}


#endif



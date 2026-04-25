/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


#include "mcu.h"
#include "rt_sm3.h"
#include "rt_crypto.h"

#include <string.h>
#include <stdio.h>


#if defined(CONFIG_SUPPORT_MULTITASKING)

extern void crypto_mutex_lock(void);
extern void crypto_mutex_unlock(void);

#else

#define crypto_mutex_lock()          ((void)0)
#define crypto_mutex_unlock()        ((void)0)

#endif

#if defined(CONFIG_CRYPTO_INT_ENABLE)
extern volatile uint32_t  sha_finish;
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i) { *((uint32_t *)(b+i)) = __REV(n); }
#endif

extern uint32_t   crypto_firmware;

extern void crypto_copy(uint32_t *p_dest_addr, uint32_t *p_src_addr, uint32_t size);

#if defined(CONFIG_CRYPTO_SM3_ENABLE)

const uint32_t sm3_init_digest[8] =
{
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

const uint32_t sm3_k[64] =
{
    0x79cc4519, 0xf3988a32, 0xe7311465, 0xce6228cb, 0x9cc45197, 0x3988a32f, 0x7311465e, 0xe6228cbc,
    0xcc451979, 0x988a32f3, 0x311465e7, 0x6228cbce, 0xc451979c, 0x88a32f39, 0x11465e73, 0x228cbce6,
    0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c, 0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
    0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec, 0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5,
    0x7a879d8a, 0xf50f3b14, 0xea1e7629, 0xd43cec53, 0xa879d8a7, 0x50f3b14f, 0xa1e7629e, 0x43cec53d,
    0x879d8a7a, 0x0f3b14f5, 0x1e7629ea, 0x3cec53d4, 0x79d8a7a8, 0xf3b14f50, 0xe7629ea1, 0xcec53d43,
    0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c, 0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
    0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec, 0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5
};

/*
 * SM3 context setup
 */
void sm3_init(sm3_context *ctx)
{

    uint32_t reg;

    ctx->total[0] = 0;
    ctx->total[1] = 0;

    crypto_mutex_lock();

    crypto_firmware = SM3_FIRMWARE;

    /*
     * check busy... almost impossible busy...
     * if busy, we should check why??
     */

    reg = CRYPTO->crypto_cfg.reg;
    if (reg & (3 << 26))
    {
        /*why? check? almost impossible*/
        while (1);        /*debug...*/
    }

    /*This initial is necessary setting.*/
    crypto_copy((uint32_t *) (CRYPTO_BASE + 0x1900), (uint32_t *) sm3_init_digest, 8);
    crypto_copy((uint32_t *) (CRYPTO_BASE + 0x1800), (uint32_t *) sm3_k, 64);

    CRYPTO->sha_k_base  = 0x00;
    CRYPTO->sha_digest_base  = 0x100;
    CRYPTO->sha_misc_cfg  =  (0x1 << 1) | (0x1);      /*SM3*/

}

static void sm3_process(uint8_t *data, uint32_t length)
{
#if defined(CONFIG_CRYPTO_INT_ENABLE)
    if (CRYPTO->crypto_cfg.bit.sha_busy)
    {
        while (sha_finish == 0)
            ;
    }

    sha_finish = 0;
#else
    /*if previous operation not finish... we should wait*/
    while (CRYPTO->crypto_cfg.bit.sha_busy)
        ;

    if (CRYPTO->crypto_cfg.bit.sha_done)
    {
        //clear the VLW_DEF register
        CRYPTO->crypto_cfg.bit.clr_crypto_int = 1;
    }
#endif

    CRYPTO->sha_dma_base  = (uint32_t) data;
    CRYPTO->sha_dma_length  = length;

    /*enable sm3*/
    CRYPTO->crypto_cfg.bit.enable_sha = 1;

    /*Notice: We don't wait SM3 finish here.*/
}

uint32_t sm3_update(sm3_context *ctx, uint8_t *input, uint32_t length)
{
    uint32_t  left, fill;
    uint32_t  temp;

    /*the following firmware check can be remove
      if code guarantee hardware resource  */
    if (crypto_firmware != SM3_FIRMWARE)
    {
        return STATUS_ERROR;
    }

    if (!length)
    {
        return STATUS_ERROR;
    }

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += length;

    if (ctx->total[0] < length)
    {
        ctx->total[1]++;
    }

    if (left && length >= fill)
    {

        memcpy((void *) (ctx->buffer + left), (void *) input, fill);

        sm3_process(ctx->buffer, 64);
        length -= fill;
        input += fill;
        left = 0;
    }

    /* in fact, length could not be more than 65536 bytes..
     * if length is more than 64KB.. separate the length several times...
     * but in real, using 64KB data for sha256 is not a good idea.
     */
    if (length >= 64)
    {
        temp = (length & ~63);       /*caculate block count.*/
        sm3_process(input, temp);
        length -= temp;
        input += temp;
    }

    if (length)
    {
        memcpy((void *) (ctx->buffer + left), (void *) input, length);
    }

    return STATUS_SUCCESS;
}

static const uint8_t sm3_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void sm3_final(sm3_context *ctx, uint8_t *digest)
{
    uint32_t last, padn;
    uint32_t high, low;
    uint8_t msglen[8];

    high = ((ctx->total[0] >> 29)
            | (ctx->total[1] << 3));
    low = (ctx->total[0] << 3);

    PUT_UINT32_BE(high, msglen, 0);
    PUT_UINT32_BE(low, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    sm3_update(ctx, (uint8_t *) sm3_padding, padn);
    sm3_update(ctx, msglen, 8);

#if defined(CONFIG_CRYPTO_INT_ENABLE)
    while (sha_finish == 0)
        ;

    sha_finish = 0;
#else

    /*wait last block finish*/
    while (!CRYPTO->crypto_cfg.bit.sha_done)
        ;

    //clear the VLW_DEF register
    CRYPTO->crypto_cfg.bit.clr_crypto_int = 1;
#endif

    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x1900)), digest, 0);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x1904)), digest, 4);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x1908)), digest, 8);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x190C)), digest, 12);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x1910)), digest, 16);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x1914)), digest, 20);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x1918)), digest, 24);
    PUT_UINT32_BE(*((uint32_t *)(CRYPTO_BASE + 0x191C)), digest, 28);

    /*clean hash value in accelerator */
    crypto_copy((uint32_t *) (CRYPTO_BASE + 0x1900), (uint32_t *) sm3_init_digest, 8);

    crypto_mutex_unlock();

}


#endif


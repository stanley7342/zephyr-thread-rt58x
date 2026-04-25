/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */



/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "mcu.h"
#include "rt_crypto.h"


extern uint32_t   crypto_firmware;

extern void crypto_start(uint8_t op_num, uint8_t sb_num);

/*Notice: interesting SM4 is little endian operation for 4 bytes!*/

#define PUT_UINT32_BE(n,b,i)     { *((uint32_t *)(b+i)) = __rev(n); }
#define GET_UINT32_BE(n,b,i)     { (n) = __rev( *((uint32_t *)((uint32_t *)(b+i)) ));}

#if defined(CONFIG_SUPPORT_MULTITASKING)

extern void crypto_mutex_lock(void);
extern void crypto_mutex_unlock(void);

#else

#define crypto_mutex_lock()          ((void)0)
#define crypto_mutex_unlock()        ((void)0)

#endif


#if defined(CONFIG_CRYPTO_SM4_ENABLE)

const uint32_t CK[32] = {
    0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
    0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
    0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
    0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
    0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
    0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
    0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
    0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

const uint32_t  sm4_genrk_bin[] = {
    0x1072496C, 0x10A20000, 0x20283E03, 0x900040D0,
    0x3018096C, 0x40007CB0, 0x3000116C, 0x12740204,
    0x520A0500, 0x528C0500, 0x528E0500, 0x52800534,
    0x6280DBFF, 0x51140500, 0x3102D80A, 0x40007DC0,
    0x121401FD, 0xAD00000D, 0x141AE773, 0xA0008006,
    0x00000000
};

const uint32_t  sm4_engine_bin[] = {
    0x1072096C, 0x10BA8947, 0x20283E03, 0x900040D0,
    0x3018016C, 0x40007CB0, 0x3000136C, 0x12740204,
    0x52420500, 0x52840500, 0x52860500, 0x52800534,
    0x529E0588, 0x529E0608, 0x6330007E, 0x6284A97F,
    0x5296C501, 0x50140500, 0x3102D80A, 0x40007DC0,
    0x121401FD, 0xAD00002B, 0x40007ED0, 0x12D7FE01,
    0xAE00003E, 0x141AE773, 0xA0008006, 0x00000000
};

const uint32_t  sm4_ecb_enc_bin[] = {
    0x10C09200, 0x181AE773, 0xA000000A, 0x10C09800,
    0x181AE773, 0xA0000028, 0xF0000000, 0x00000000
};

const uint32_t  sm4_ecb_dec_bin[] = {
    0x10C0A600, 0x181AE773, 0xA000000A, 0x10BA0967,
    0x20283E1F, 0x40007ED0, 0x12B803FF, 0xAD000055,
    0x10BA0948, 0x40007DE0, 0x12B80201, 0xAE000059,
    0x10C0BE00, 0x181AE773, 0xA0000028, 0xF0000000
};

const uint32_t FK[4] = {
    0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC
};

void sm4_fw_init(void) {
    /*TODO: mutex lock ? */

    crypto_copy((uint32_t*) (CRYPTO_BASE + 0x1C90), (uint32_t*) CK, 32);

    crypto_copy((uint32_t*) (CRYPTO_BASE + 0x1028), (uint32_t*) sm4_genrk_bin,
                sizeof(sm4_genrk_bin));

    crypto_copy((uint32_t*) (CRYPTO_BASE + 0x10A0), (uint32_t*) sm4_engine_bin,
                sizeof(sm4_engine_bin));

    crypto_copy((uint32_t*) (CRYPTO_BASE + 0x1118), (uint32_t*) sm4_ecb_enc_bin,
                sizeof(sm4_ecb_enc_bin));

    crypto_copy((uint32_t*) (CRYPTO_BASE + 0x1140), (uint32_t*) sm4_ecb_dec_bin,
                sizeof(sm4_ecb_dec_bin));

}


void sm4_ecb_encode(uint8_t* out_packet, uint8_t* in_packet, uint8_t* mkey) {
    uint32_t i;
    uint32_t key[4];
    uint32_t* temp_in_ptr, *temp_out_ptr, *temp_key_ptr, *temp_mkey_ptr;

    /*in fact, sm4 operation is little endian */

    /* in fact, even if in_packet is not 4 byte alignment,
     * cortex-m3 corss-boundary access is available
     */
    temp_in_ptr = (uint32_t*) (in_packet);
    temp_out_ptr = (uint32_t*) (out_packet);
    temp_key_ptr = (uint32_t*) (key);
    temp_mkey_ptr = (uint32_t*)  (mkey);

    for (i = 0; i <= 3; i++) {
        temp_out_ptr[i] = __REV( temp_in_ptr[i]);
        temp_key_ptr[i] =  __REV( temp_mkey_ptr[i]);
        key[i] = key[i] ^ FK[i];
    }

    crypto_copy((uint32_t*)(CRYPTO_BASE + 0x1D10), (uint32_t*)temp_out_ptr, 4);

    crypto_copy((uint32_t*)(CRYPTO_BASE + 0x1C00), (uint32_t*)key, 4);

    *((volatile uint32_t*) (CRYPTO_BASE + 0x1000)) = (0xa0000000 |
                                                      SM4_ECB_Enc_inst_index);

#if defined(CONFIG_CRYPTO_INT_ENABLE)
    crypto_start(3, 0);

    while (crypto_finish == 0)
        ;

    crypto_finish = 0;

#else
    crypto_start(3, 0);

    //Waiting for calculation done
    while (!CRYPTO->crypto_cfg.bit.crypto_done);

    //clear the VLW_DEF register
    CRYPTO->crypto_cfg.bit.clr_crypto_int = 1;
#endif

    /*copy the result*/
    crypto_copy((uint32_t*)out_packet, (uint32_t*)(CRYPTO_BASE + 0x1D10), 4);

    /*return the result to big endian*/
    for (i = 0; i <= 3; i++) {
        temp_out_ptr[i] = __REV(temp_out_ptr[i]);
    }
}


void sm4_ecb_decode(uint8_t* out_packet, uint8_t* in_packet, uint8_t*  mkey) {
    uint32_t i;
    uint32_t key[4];
    uint32_t* temp_in_ptr, *temp_out_ptr, *temp_key_ptr, *temp_mkey_ptr;

    /*in fact, sm4 operation is little endian */

    /* in fact, even if in_packet is not 4 byte alignment,
     * cortex-m3 corss-boundary access is available
     */
    temp_in_ptr = (uint32_t*) (in_packet);
    temp_out_ptr = (uint32_t*) (out_packet);
    temp_key_ptr = (uint32_t*) (key);
    temp_mkey_ptr = (uint32_t*)  (mkey);

    for (i = 0; i <= 3; i++) {
        temp_out_ptr[i] = __REV( temp_in_ptr[i]);
        temp_key_ptr[i] =  __REV( temp_mkey_ptr[i]);
        key[i] = key[i] ^ FK[i];
    }

    crypto_copy((uint32_t*)(CRYPTO_BASE + 0x1D10), (uint32_t*)temp_out_ptr, 4);

    crypto_copy((uint32_t*)(CRYPTO_BASE + 0x1C00), (uint32_t*)key, 4);

    *((volatile uint32_t*) (CRYPTO_BASE + 0x1000)) = (0xa0000000 |
                                                      SM4_ECB_Dec_inst_index);

#if defined(CONFIG_CRYPTO_INT_ENABLE)
    crypto_start(3, 0);

    while (crypto_finish == 0)
        ;

    crypto_finish = 0;
#else
    crypto_start(3, 0);


    //Waiting for calculation done
    while (!CRYPTO->crypto_cfg.bit.crypto_done);

    //clear the VLW_DEF register
    CRYPTO->crypto_cfg.bit.clr_crypto_int = 1;
#endif

    /*copy the result*/
    crypto_copy((uint32_t*)out_packet, (uint32_t*)(CRYPTO_BASE + 0x1D10), 4);

    /*return the result to big endian*/
    for (i = 0; i <= 3; i++) {
        temp_out_ptr[i] = __REV(temp_out_ptr[i]);
    }
}

uint32_t sm4_acquire(void) {
    /*Not implement yet.. This function is reserved for  Multitasking used*/
    crypto_mutex_lock();

    if (crypto_firmware != SM4_FIRMWARE) {
        sm4_fw_init();   /*reload SM4 firmware */
        crypto_firmware = SM4_FIRMWARE;
    }

    return STATUS_SUCCESS;
}

uint32_t sm4_release(void) {
    /*Not implement yet.. This function is reserved for Multitasking used*/

    if (crypto_firmware != SM4_FIRMWARE) {
        return STATUS_ERROR;
    }

    crypto_mutex_unlock();

    return STATUS_SUCCESS;
}

#endif


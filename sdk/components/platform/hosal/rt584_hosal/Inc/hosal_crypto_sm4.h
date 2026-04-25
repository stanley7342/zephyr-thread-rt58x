/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_sm4.c
 * \brief           hosal_crytpo_sm4 file
 */

/*
 * This file is part of library_name.
 * Author:
 */

#ifndef HOSAL_CRYPTO_SM4_H
#define HOSAL_CRYPTO_SM4_H

#include "mcu.h"
#include "rt_crypto_util.h"
#include "rt_crypto.h"
#include "rt_sm4.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
 * \defgroup        HOSAL_CRYPTO_SM4 Hosal crypto sm4
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal crypto sm4 definitions, structures, and functions
 * @{
 */


/**
 * \brief           sm4 operation encode/decode operation devine
 */
typedef  enum {
    HOSAL_SM4_NULL = 0,
    HOSAL_SM4_ENCODE = 1,
    HOSAL_SM4_DECODE = 2,
} hosal_crypto_sm4_operation_t;

/**
 * \brief           sm4 operation in/out point  and input length, 
 */
typedef struct {
    uint8_t*     mkey_ptr;
    uint8_t*     in_ptr;
    uint8_t*     out_ptr;
    uint32_t     loop;
    hosal_crypto_sm4_operation_t    crypto_operation;
} hosal_sm4_dev_t;

/**
 * \brief           hosal sm4 intintal function
 */
int hosal_crypto_init(void) ;

/**
 * \brief           hosal sm4 operation function
 */
int hosal_crypto_sm4_operation(hosal_sm4_dev_t* sm4_dev);

/*@}*/ /* end of RT584_HOSAL HOSAL_CRYPTO_SM4 */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_SM4_H */

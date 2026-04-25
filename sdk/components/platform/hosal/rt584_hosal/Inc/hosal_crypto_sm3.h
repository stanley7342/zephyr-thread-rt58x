/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_sm3.c
 * \brief           hosal_crytpo_sm3 file
 */

/*
 * This file is part of library_name.
 * Author:
 */

#ifndef HOSAL_CRYPTO_SHA256_H
#define HOSAL_CRYPTO_SHA256_H

#include "mcu.h"
#include "rt_crypto.h"
#include "rt_sm3.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup        HOSAL_CRYPTO_SM3 Hosal crypto sm3
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal crypto sm3 definitions, structures, and functions
 * @{
 */

/**
 * \brief           sm3 operation enum define
 */
typedef  enum {
    HOSAL_SM3_NULL   = 0,
    HOSAL_SM3_DIGEST = 1,
    HOSAL_SM3_INIT = 2,
    HOSAL_SM3_UPDATE = 3,
    HOSAL_SM3_FINAL = 4,
} hosal_crypto_sm3_operation_t;

/**
 * \brief           sm3 operation in/out point  and input length, sm3 context define
 */
typedef struct {
    uint8_t*     in_ptr;
    uint8_t*     out_ptr;
    uint32_t     in_length;
    sm3_context  sm3_cnxt;
    hosal_crypto_sm3_operation_t    crypto_operation;
} hosal_sm3_dev_t;

/**
 * \brief           hosal sm3 operation function
 */
int hosal_crypto_sm3_operation(hosal_sm3_dev_t* sm3_dev);

/*@}*/ /* end of RT584_HOSAL HOSAL_CRYPTO_SM3 */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_SM3_H */

/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_ctr_drbg.h
 * \brief           hosal_crytpo_ctr_drbg include file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#ifndef HOSAL_CRYPTO_CTR_DRBG_H
#define HOSAL_CRYPTO_CTR_DRBG_H

#include "mcu.h"
#include "rt_crypto.h"
#include "rt_sha256.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_CRYPTO_CTR_DRBG Hosal crypto ctr drbg
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal crypto ctr drbg definitions, structures, and functions
 * @{
 */


/**
 * \brief           hosal crypto curve c25519 const defined
 */
#define HOSAL_CURVE_C25519_INIT             1

/**
 * \brief           hosal crypto aes struct defined
 */
typedef  enum {
    HOSAL_CURVE_NULL = 0,         /*!< CURVE operation value defineds   */
    HOSAL_CURVE_C25519_MUL = 1,   /*!< CURVE operation value defineds   */
} hosal_crypto_operation_t;


/**
 * \brief           hosal crypto curve 25519 operation struct defined
 */
typedef struct {
    uint32_t* blind_zr;
    uint32_t* public_key;
    uint32_t* secret_key;
    uint32_t* base_point;
    hosal_crypto_operation_t crypto_operation;
} hosal_crypto_curve25519_t;

/**
 * \brief           hosal_crypto_curve_c25519_init
 * \param[in]       NONE
 * \return          return function status
 */
int hosal_crypto_curve_c25519_init(void);

/**
 * \brief           hosal_crypto_curve25519_operation
 * \param[in]       curve25519 cureve 25519 intput value;
 * \return          return function status
 */
int hosal_crypto_curve25519_operation(hosal_crypto_curve25519_t* curve25519);

/**
 * \brief           hosal_crypto_curve_c25519_release
 * \param[in]       NONE
 * \return          return function status
 */
int hosal_crypto_curve_c25519_release(void) ;

/*@}*/ /* end of RT58X_HOSAL HOSAL_CRYPTO_CTR_DRBG */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_CTR_DRBG_H */

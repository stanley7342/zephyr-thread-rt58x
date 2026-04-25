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
#include "rt_ctr_drbg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
 * \defgroup        HOSAL_CRYPTO_CTR_DRBG Hosal crypto ctr drbg
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal crypto ctr drbg definitions, structures, and functions
 * @{
 */

/**
 * \brief           hosal crypto curve c25519 const defined
 */
#define HOSAL_CURVE_C25519_INIT             1

/**
 * \brief           ctr drbg enum defined
 */
typedef  enum {
    HOSAL_CTR_DRBG_NULL = 0,                    /*!< ctr drbg operation null   */
    HOSAL_CTR_DRBG_INI = 1,                     /*!< ctr drbg operation inttinal   */
    HOSAL_CTR_DRBG_PR_ON = 2,                   /*!< ctr drbg operation pr on   */
    HOSAL_CTR_DRBG_RANDOM = 3,                  /*!< ctr drbg operation random   */
    HOSAL_CTR_DRBG_RANDOM_ADD = 4,              /*!< ctr drbg operation random add   */
    HOSAL_CTR_DRBG_RESEED = 5,                  /*!< ctr drbg operation reseed   */
    HOSAL_CURVE_C25519_MUL = 6,                 /*!< curve c25519 operation mul   */
    HOSAL_CURVE_C25519_RELEASE = 7,             /*!< curve c25519 operation release   */
} hosal_crypto_operation_t;

/**
 * \brief           ctr drbg operation struce defined
 */
typedef struct {

    /*
    * Callbacks (Entropy)
    */
    uint32_t (*f_entropy)(void*, uint8_t *, uint32_t);       /*!<  the entropy callback function.
                                                                NULL for using default TRNG to generate entropy data.
                                                                */
    void*                    p_entropy;
    union {
        uint8_t*                 custom;
        uint8_t*                 output_ptr;
    };
    uint8_t*                  additional;
    union {
        uint32_t                   len;
        uint32_t                   output_len;
    };
    uint32_t                    add_len;
    ctr_drbg_context            ctx;
    hosal_crypto_operation_t    crypto_operation;
} hosal_ctr_drbg_t;

/**
 * \brief           curve25519 operation struce defined
 */
typedef struct {

    uint32_t* blind_zr;                         /*!<  */
    uint32_t* public_key;                       /*!<  */
    uint32_t* secret_key;                       /*!<  */
    uint32_t* base_point;                       /*!<  */
    hosal_crypto_operation_t crypto_operation;  /*!<  */
} hosal_crypto_curve25519_t;

/**
 * \brief           ctr debg operation function
 * \return          function status;
 */
int hosal_crypto_ctr_drbg_operation(hosal_ctr_drbg_t* ctr_drbg_dev);

/**
 * \brief           curve c25519 initinal function
 * \return          function status;
 */
int hosal_crypto_curve_c25519_init(void);

/**
 * \brief           curve c25519 operation function
 * \return          function status;
 */
int hosal_crypto_curve25519_operation(hosal_crypto_curve25519_t* curve25519);

/**
 * \brief           curve c25519 release function
 * \return          function status;
 */
int hosal_crypto_curve_c25519_release(void);

/**
 * \brief           ctr debg inttinal function
 * \return          function status;
 */
int hosal_ctr_drbg_init(void);

/*@}*/ /* end of RT584_HOSAL  HOSAL_CRYPTO_CTR_DRBG */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_CTR_DRBG_H */

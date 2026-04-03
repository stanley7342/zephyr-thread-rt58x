/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_ecc.h
 * \brief           hosal_crytpo_ecc include file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#ifndef HOSAL_CRYPTO_ECC_H
#define HOSAL_CRYPTO_ECC_H

#include "rt_crypto.h"
#include "rt_ecc.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_CRYPTO_ECC Hosal crypto ecc
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal crypto ecc definitions, structures, and functions
 * @{
 */

/**
 * \brief           hosal crypto ecc const defined
 */
#define HOSAL_NULL                          0
#define HOSAL_ECC_CURVE_P256_INIT           1
#define HOSAL_ECDA_CURVE_P256_VERIFY_INIT   2
#define HOSAL_ECC_CURVE_P192_INIT           3
#define HOSAL_ECC_CURVE_B163_INIT           4
/**
 * \brief           hosal_crypto_ecc_release enum value defined
 */
typedef  enum {
    HOSAL_ECDA_P256_SIGNATURE = 1,
    HOSAL_ECDA_P256_VERIFY = 2,
    HOSAL_GFP_P256_MULTI = 3,
    HOSAL_GFP_P256_VAILD_VERIFY =4,
    HOSAL_GFP_P256_ADD = 5,
    HOSAL_GFP_P256_MOD_MULTI = 6,
    HOSAL_GFP_P192_MULTI = 7,
    HOSAL_GFP_B163_MULTI = 8,
} hosal_crypto_ecc_operation_t;

/**
 * \brief           hosal_crypto_ecc_p256 struct defined
 */
typedef struct {
    Signature_P256* signatrue;
    union {
        uint32_t* p_hash;
        uint32_t* p_hash_message;
        uint32_t* p_result;
    };
    union {
        uint32_t* p_key;
        uint32_t* p_x;
        uint32_t* p_result_x;
    };
    union {
        uint32_t* p_k;
        uint32_t* p_y;
    };
    union {
        ECPoint_P256* p_signatrue;
        ECPoint_P256* result;
        ECPoint_P256* p_point_result;
    };
    union {
        ECPoint_P256* base;
        ECPoint_P256* p_point_x1;
        ECPoint_P256* p_public_key;
    };
    ECPoint_P256* p_point_x2;
    hosal_crypto_ecc_operation_t crypto_operation;
} hosal_crypto_ecc_p256_t;


/**
* \brief hosal_crypto_ecc_p256_add_t struct
*/
typedef struct {

    ECPoint_P256* p_point_result;
    ECPoint_P256* p_point_x1;
    ECPoint_P256* p_point_x2;
    hosal_crypto_ecc_operation_t crypto_operation;
} hosal_crypto_ecc_p256_add_t;
/**
 * \brief           hosal_crypto_ecc_gf_add struct defined
 */
typedef struct {
    uint32_t* p_result_x;
    uint32_t* p_result_y;
    uint32_t* target_x;
    uint32_t* target_y;
    uint32_t* target_k;
    hosal_crypto_ecc_operation_t crypto_operation;
} hosal_crypto_ecc_gf_t;
/**
 * \brief           hosal_crypto_ecc_init
 * \param[in]       ctl control command
 * \return          return function status
 */
int hosal_crypto_ecc_init(int ctl);
/**
 * \brief           hosal_crypto_ecc_p256
 * \param[in]       ecc_p256 ecc operation struct
 * \return          return function status
 */
int hosal_crypto_ecc_p256(hosal_crypto_ecc_p256_t* ecc_p256);
/**
 * \brief           hosal_crypto_ecc_gf_operation
 * \param[in]       ecc_gf ecc_gf operation struct
 * \return          return function status
 */
int hosal_crypto_ecc_gf_operation(hosal_crypto_ecc_gf_t* ecc_gf);
/**
 * \brief           hosal_crypto_ioctl
 * \param[in]       ctl control command
 * \param[in]       p_arg paramater
 * \return          return function status
 */
int hosal_crypto_ioctl(int ctl, void* p_arg);

/*@}*/ /* end of RT58X_HOSAL HOSAL_CRYPTO_ECC */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_ECC_H */

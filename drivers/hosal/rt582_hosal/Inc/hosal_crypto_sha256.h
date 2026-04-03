/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_sha256.h
 * \brief           hosal_crytpo_sha256 include file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#ifndef HOSAL_CRYPTO_SHA256_H
#define HOSAL_CRYPTO_SHA256_H

#include "mcu.h"
#include "rt_crypto.h"
#include "rt_sha256.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_CRYPTO_SHA256 Hosal crypto sha256
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal crypto sha256 definitions, structures, and functions
 * @{
 */


/**
* \brief           HOSAL SHA256 const define
*/
#define HOSLA_CRYPTO_SHA256_INI   2
#define HOSAL_SHA256_DIGEST_SIZE  SHA256_DIGEST_SIZE
#define HOSAL_SHA256_BLOCK_SIZE   SHA256_BLOCK_SIZE
#define HOSAL_NUMBER_OF_GENERATES 2

/**
* \brief           HOSAL SHA256 operation enum type
*/
typedef  enum {
    HOSAL_SHA256_DIGEST_INIT = 0,
    HOSAL_SHA256_DIGEST_STARTS = 1,
    HOSAL_SHA256_DIGEST_UPDATE = 2,
    HOSAL_SHA256_DIGEST_FINISH = 3,
    HOSAL_SHA256_DIGEST = 4,
    HOSAL_SHA256_HMAC = 5,
    HOSAL_SHA256_HKDF = 6,
    HOSAL_SHA256_PBKDF2_HMAC = 7,
} hosal_crypto_sha256_operation_t;

/**
* \brief           HOSAL SHA256 operation struct defined
*/
typedef struct {
    uint8_t* key_ptr;
    uint8_t* in_ptr;
    uint8_t* out_ptr;
    uint8_t* secret;
    uint8_t* salt;
    uint8_t* info;
    uint32_t key_length;
    uint32_t in_length;
    uint32_t out_len;
    uint32_t secret_len;
    uint32_t salt_len;
    uint32_t info_len;
    pbkdf2_st* pbkdf2;
    hosal_crypto_sha256_operation_t crypto_operation;
    sha256_context sha_cnxt;
} hosal_sha256_dev_t;

/**
 * \brief           hosal_crypto_sha256_init
 * \param[in]       NONE
 * \return          return function status
 */
int hosal_crypto_sha256_init(void);

/**
 * \brief           hosal_crypto_sha256_operation
 * \param[in]       sha256_dev sha256  operation struct
 * \return          return function status
 */
int hosal_crypto_sha256_operation(hosal_sha256_dev_t* sha256_dev);

/*@}*/ /* end of RT58X_HOSAL HOSAL_CRYPTO_SHA256 */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_SHA256_H */

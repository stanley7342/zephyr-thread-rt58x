/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_ecjpake.h
 * \brief           hosal_crytpo_ecjpake include file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#ifndef HOSAL_CRYPTO_ECJPAKE_H
#define HOSAL_CRYPTO_ECJPAKE_H

#include "mcu.h"
#include "rt_crypto.h"
#include "rt_ecjpake.h"
#include "rt_sha256.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_CRYPTO_ECJPAKE Hosal crypto ecjpake
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal crypto ecjpake definitions, structures, and functions
 * @{
 */


/**
 * \brief           hosal_crypto_ecjpack const defined
 */
#define HOSLA_CRYPTO_ECJPAKE_INI  1
#define HOSAL_NUMBER_OF_GENERATES 2

/**
 * \brief           hosal crypto ecjpack operation struct dfeindd
 */
typedef enum {
    HOSAL_ECJPAKE_NULL = 0,
    HOSAL_ECJPAKE_GENERATE_ZKP = 1,
    HOSAL_ECJPAKE_GENERATE_VERIFY = 2,
    HOSAL_ECJPAKE_GENERATE_ZKP_2 = 3,
    HOSAL_ECJPAKE_GENERATE_VERIFY_2 = 4,
    HOSAL_ECJPAKE_COMPUTE_KEY = 5,
} hosal_crypto_ecjpake_operation_t;

/**
 * \brief           hosal crypto ecjpack struct dfeindd
 */
typedef struct {
    ECJPAKE_CTX* ctx;
    ECJPAKEKeyKP* key;
    ECPoint_P256* gen;
    uint8_t* private_key;
    uint8_t* pms;
    hosal_crypto_ecjpake_operation_t crypto_operation;
} hosal_ecjpake_dev_t;

/**
 * \brief           hosal_crypto_ecjpake_init
 * \param[in]       NONE
 * \return          return function status
 */
int hosal_crypto_ecjpake_init(void);

/**
 * \brief           hosal_crypto_ecjpake_operation
 * \param[in]       ecjpake_dev ecjpake operation struct
 * \return          return function status
 */
int hosal_crypto_ecjpake_operation(hosal_ecjpake_dev_t* ecjpake_dev);

/*@}*/ /* end of RT58X_HOSAL HOSAL_CRYPTO_ECJPAKE */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_ECJPAKE_H */

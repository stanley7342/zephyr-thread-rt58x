/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_hmac_drbg.h
 * \brief           hosal_crytpo_hmac_drbg include file
 */

/*
 * This file is part of library_name.
 * Author:
 */

#ifndef HOSAL_CRYPTO_HMAC_DRBG_H
#define HOSAL_CRYPTO_HMAC_DRBG_H

#include "mcu.h"
#include "rt_crypto.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
 * \defgroup        HOSAL_CRYPTO_HMAC_DRBG Hosal crypto hmac drbg
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal crypto hmac drbg definitions, structures, and functions
 * @{
 */


/**
 * \brief           Crypto generates number const defined
 */
#define HOSAL_NUMBER_OF_GENERATES   2

/**
 * \brief           hosal hamc drbg operation enum define
 */
typedef  enum {
    HOSAL_HMAC_DRBG_HMAC = 1,
} hosal_crypto_hmac_drbg_operation_t;

/**
 * \brief           hmac drgb operation struct defined
 */
typedef struct {
    uint8_t*    seed_material;
    uint8_t*     in_ptr;
    uint8_t*     out_ptr;
    uint8_t*     data_ptr;
    uint32_t     seed_material_length;
    uint32_t     in_length;
    uint32_t     out_len;
    uint32_t     data_len;
    hmac_drbg_state hmac_drbg_ctx;
    hosal_crypto_hmac_drbg_operation_t  crypto_operation;
} hosal_hmac_drbg_dev_t;

/**
 * \brief           hosal_crypto_hmac_drbg_init
 * \param[in]       NONE
 * \return          return function status
 */
int hosal_crypto_hmac_drbg_init(void);

/**
 * \brief           hosal_crypto_hmac_drbg_operation
 * \param[in]       hmac_drbg hmac drbg struct defineds
 * \return          return function status
 */
int hosal_crypto_hmac_drbg_operation(hosal_hmac_drbg_dev_t* hmac_drbg);

/**
 * \brief           hosal_hmac_drbg_generate_k
 * \param[in]       hmac_drbg hmac drbg struct defineds
 * \return          return function status
 */
int hosal_hmac_drbg_generate_k(hosal_hmac_drbg_dev_t* hmac_drbg);

/**
 * \brief           hosal_hmac_drbg_update
 * \param[in]       state hmac drbg update status
 * \param[in]       data hmac drbg update data
 * \param[in]       data_length hmac drbg update data length
 * \return          return function status
 */
void  hosal_hmac_drbg_update(hmac_drbg_state* state, uint8_t* data,
                             uint32_t  data_length);

/**
 * \brief           hosal_hmac_drbg_generate
 * \param[in]       out_result hmac drbg generate output buffer point
 * \param[in]       state hmac drbg generate data
 * \param[in]       request_bytes hmac drbg generate retuest bytes
 * \param[in]       data hmac drbg generate data
 * \param[in]       data_length hmac drbg generate data length 
 * \return          return function status
 */
void  hosal_hmac_drbg_generate(uint8_t* out_result, hmac_drbg_state* state,
                               uint32_t request_bytes, uint8_t* data, uint32_t data_length);

/**
 * \brief           hosal_hmac_drbg_instantiate
 * \param[in]       state hmac drbg instantiate status
 * \param[in]       seed_material hmac drbg instantiate seed material buffer point.
 * \param[in]       seed_material_length hmac drbg instantiate seed material length.
 * \return          return function status
 */
uint32_t hosal_hmac_drbg_instantiate(hmac_drbg_state* state,
                                     uint8_t* seed_material, uint32_t seed_material_length);


/*@}*/ /* end of RT584_HOSAL HOSAL_CRYPTO_HMAC_DRBG */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_CRYPTO_HMAC_DRBG_H */

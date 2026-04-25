/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

 /**
 * \file           rt_hmac_drbg.h
 * \brief          
 */

#ifndef _RT584_HMAC_DRBG_H_
#define _RT584_HMAC_DRBG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rt_crypto.h"


/**
 * \defgroup        HMAC_DRBG Hmac drbg
 * \ingroup         RT584_DRIVER
 * \brief           DefineHmac drbg definitions, structures, and functions
 * @{
 */


typedef struct hmac_drbg_state_struct {
    uint8_t key[SHA256_DIGEST_SIZE];
    uint8_t v[SHA256_DIGEST_SIZE];
} hmac_drbg_state;

/**
* \brief
* \param[in]  None
* \details
*/
uint32_t hmac_drbg_instantiate(hmac_drbg_state *state, uint8_t *seed_material, uint32_t seed_material_length);

/**
* \brief
* \param[in]  None
* \details
*/
/*data is  (entropy_input||additional_input) */
void  hmac_drbg_update(hmac_drbg_state *state, uint8_t *data, uint32_t data_length);

/**
* \brief
* \param[in]  None
* \details
*/
/*
 * NIST SP800-90A page 47 HMAC_DRBG_Generate_algorithm
 * data is additional input, it could be NULL.
 */
void  hmac_drbg_generate(uint8_t *out_result, hmac_drbg_state *state, uint32_t request_bytes,
                         uint8_t *data, uint32_t data_length);

/*@}*/ /* end of RT584_DRIVER HMAC_DRBG */

#ifdef __cplusplus
}
#endif

#endif   /* End of _RT584_HMAC_DRBG_H_ */

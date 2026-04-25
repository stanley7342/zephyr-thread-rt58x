/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_ecc.c
 * \brief           hosal_crytpo_ecc file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mcu.h"
#include "rt_crypto.h"
#include "rt_ecc.h"
#include "hosal_crypto_ecc.h"
#include "hosal_status.h"


int hosal_crypto_ecc_init(int ctl) {

    int status = STATUS_SUCCESS;

    switch (ctl) {
        case HOSAL_ECC_CURVE_P256_INIT:

            gfp_ecc_curve_p256_init();

            break;

        case HOSAL_ECDA_CURVE_P256_VERIFY_INIT:

            gfp_ecdsa_p256_verify_init();

            break;

        case HOSAL_ECC_CURVE_P192_INIT:

            gfp_ecc_curve_p192_init();

            break;

        case HOSAL_ECC_CURVE_B163_INIT:

            gf2m_ecc_curve_b163_init();

            break;

        case HOSAL_ECC_CURVE_SM2P192_INIT:

            gfp_ecc_curve_sm2p192_init();

            break;

        case HOSAL_ECC_CURVE_SM2P256_INIT:

            gfp_ecc_curve_sm2p256_init();

            break;
    }

    return status;
}

int hosal_crypto_ecc_p256(hosal_crypto_ecc_p256_t* ecc_p256) {

    int status = STATUS_SUCCESS;

    if (ecc_p256->crypto_operation == HOSAL_ECDA_P256_SIGNATURE) {


        status = gfp_ecdsa_p256_signature(ecc_p256->signatrue, ecc_p256->p_hash,
                                          ecc_p256->p_key, ecc_p256->p_k);


    } else if (ecc_p256->crypto_operation == HOSAL_ECC_SM2P256_SIGNATURE) {

        status = gfp_ecdsa_sm2p256_signature(ecc_p256->sm2_signatrue, ecc_p256->p_hash,
                                             ecc_p256->p_key, ecc_p256->p_k);

    } else if (ecc_p256->crypto_operation == HOSAL_ECDA_P256_VERIFY) {

        status = gfp_ecdsa_p256_verify(ecc_p256->signatrue, ecc_p256->p_hash,
                                       ecc_p256->base);

    } else if (ecc_p256->crypto_operation == HOSAL_ECDA_SM2P256_VERIFY) {

        status = gfp_ecdsa_sm2p256_verify(ecc_p256->p_result_x, ecc_p256->p_signatrue,
                                          ecc_p256->p_hash_message, ecc_p256->p_public_key);

    } else if (ecc_p256->crypto_operation == HOSAL_GFP_P256_VAILD_VERIFY) {
        status = gfp_valid_point_p256_verify( ecc_p256->result);
    } else if (ecc_p256->crypto_operation == HOSAL_GFP_P256_MULTI) {
        status = gfp_point_p256_mult(ecc_p256->result, ecc_p256->base, ecc_p256->p_key);
    } else if (ecc_p256->crypto_operation == HOSAL_GFP_P256_ADD) {
        status = gfp_point_p256_add(ecc_p256->p_point_result, ecc_p256->p_point_x1,
                                    ecc_p256->p_point_x2);
    } else if (ecc_p256->crypto_operation == HOSAL_GFP_P256_MOD_MULTI) {
        gfp_scalar_modmult_p256(ecc_p256->reseult, ecc_p256->p_x, ecc_p256->p_y);
    } else {
        return STATUS_INVALID_PARAM;
    }

    return status;
}

int hosal_crypto_ecc_gf_operation(hosal_crypto_ecc_gf_t* ecc_gf) {

    int status = STATUS_SUCCESS;

    if (ecc_gf->crypto_operation == HOSAL_GFP_P192_MULTI) {
        status = gfp_point_p192_mult(ecc_gf->p_result_x, ecc_gf->p_result_y,
                                     ecc_gf->target_x, ecc_gf->target_y, ecc_gf->target_k);
    } else if (ecc_gf->crypto_operation == HOSAL_GFP_B163_MULTI) {

        status = gf2m_point_b163_mult(ecc_gf->p_result_x, ecc_gf->p_result_y,
                                      ecc_gf->target_x, ecc_gf->target_y, ecc_gf->target_k);

    }

    return status;
}








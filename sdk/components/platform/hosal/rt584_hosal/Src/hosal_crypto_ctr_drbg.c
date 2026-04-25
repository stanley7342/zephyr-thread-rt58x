/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_ctr_drbg.c
 * \brief           hosal_crytpo_ctr_drbg file
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
#include "rt_ctr_drbg.h"
#include "hosal_crypto_ctr_drbg.h"




int hosal_ctr_drbg_init(void) {

    int status = STATUS_SUCCESS;

    crypto_lib_init();

    return status;
}

int hosal_crypto_curve_c25519_init(void) {

    int status = STATUS_SUCCESS;

    curve_c25519_init();

    return status;
}

int hosal_crypto_curve_c25519_release(void) {

    int status = STATUS_SUCCESS;

    curve25519_release();

    return status;
}

int hosal_crypto_curve25519_operation(hosal_crypto_curve25519_t* curve25519) {

    int status = STATUS_SUCCESS;


    if (curve25519->crypto_operation == HOSAL_CURVE_C25519_MUL) {
        curve25519_point_mul(curve25519->blind_zr, curve25519->public_key,
                             curve25519->secret_key, curve25519->base_point);
    } else {
        status = STATUS_INVALID_PARAM;
    }

    return status;
}


int hosal_crypto_ctr_drbg_init(void) {

    int status = STATUS_SUCCESS;

    crypto_lib_init();

    return status;
}


int hosal_crypto_ctr_drbg_operation(hosal_ctr_drbg_t* ctr_drbg) {

    int status;

    status = STATUS_SUCCESS;

    if (ctr_drbg->crypto_operation == HOSAL_CTR_DRBG_INI) {
        status = ctr_drbg_init( &ctr_drbg->ctx, ctr_drbg->f_entropy, ctr_drbg->p_entropy,
                                ctr_drbg->custom, ctr_drbg->len);
    } else if (ctr_drbg->crypto_operation == HOSAL_CTR_DRBG_PR_ON) {
        ctr_drbg_set_prediction_resistance( &ctr_drbg->ctx, CTR_DRBG_PR_ON );
    } else if (ctr_drbg->crypto_operation == HOSAL_CTR_DRBG_RANDOM) {
        status =  ctr_drbg_random( &ctr_drbg->ctx, ctr_drbg->output_ptr, ctr_drbg->output_len);
    } else if (ctr_drbg->crypto_operation == HOSAL_CTR_DRBG_RANDOM_ADD) {
        status =  ctr_drbg_random_with_add( &ctr_drbg->ctx, ctr_drbg->output_ptr,
                                            ctr_drbg->output_len, ctr_drbg->additional, ctr_drbg->add_len);
    } else if (ctr_drbg->crypto_operation == HOSAL_CTR_DRBG_RESEED) {
        status =  ctr_drbg_reseed( &ctr_drbg->ctx, ctr_drbg->additional, ctr_drbg->add_len);
    } else {
        status = STATUS_INVALID_PARAM;
    }

    return status;
}








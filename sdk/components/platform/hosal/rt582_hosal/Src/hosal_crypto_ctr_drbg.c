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
#include "hosal_crypto_ctr_drbg.h"
#include "hosal_status.h"


int hosal_crypto_curve_c25519_init(void) {

    int status = HOSAL_STATUS_SUCCESS;

    curve_c25519_init();

    return status;
}

int hosal_crypto_curve_c25519_release(void) {

    int status = HOSAL_STATUS_SUCCESS;

    curve25519_release();

    return status;
}

int hosal_crypto_curve25519_operation(hosal_crypto_curve25519_t* curve25519) {

    int status = HOSAL_STATUS_SUCCESS;

    if (curve25519->crypto_operation == HOSAL_CURVE_C25519_MUL) {
        curve25519_point_mul(curve25519->blind_zr, curve25519->public_key,
                             curve25519->secret_key, curve25519->base_point);
    } else {
        status = HOSAL_STATUS_INVALID_PARAM;
    }

    return status;
}

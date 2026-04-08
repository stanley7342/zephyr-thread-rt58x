/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_ecjpake.c
 * \brief           hosal_crytpo_ecjpake file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hosal_crypto_ecjpake.h"
#include "hosal_status.h"
#include "mcu.h"

int hosal_crypto_ecjpake_init(void) {
    int status = HOSAL_STATUS_SUCCESS;

    sha256_vector_init();

    return status;
}

int hosal_crypto_ecjpake_operation(hosal_ecjpake_dev_t* ecjpake_dev) {
    int status;

    status = HOSAL_STATUS_SUCCESS;
    if (ecjpake_dev->crypto_operation == HOSAL_ECJPAKE_GENERATE_ZKP) {

        ecjpake_generate_zkp(ecjpake_dev->ctx, ecjpake_dev->key,
                             ecjpake_dev->gen, ecjpake_dev->private_key);

    } else if (ecjpake_dev->crypto_operation == HOSAL_ECJPAKE_GENERATE_VERIFY) {

        status = ecjpake_verify_zkp(ecjpake_dev->ctx, ecjpake_dev->gen,
                                    ecjpake_dev->key);
    } else if (ecjpake_dev->crypto_operation == HOSAL_ECJPAKE_GENERATE_ZKP_2) {
        status = ecjpake_generate_step2_zkp(ecjpake_dev->ctx, ecjpake_dev->key);

    } else if (ecjpake_dev->crypto_operation
               == HOSAL_ECJPAKE_GENERATE_VERIFY_2) {

        status = ecjpake_verify_step2_zkp(ecjpake_dev->ctx, ecjpake_dev->key);

    } else if (ecjpake_dev->crypto_operation == HOSAL_ECJPAKE_COMPUTE_KEY) {
        status = ecjpake_compute_key(ecjpake_dev->ctx, ecjpake_dev->key,
                                     ecjpake_dev->pms);
    }

    /*server ecjpake ctx*/

    return status;
}

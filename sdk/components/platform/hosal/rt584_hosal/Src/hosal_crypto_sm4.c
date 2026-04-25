/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_sm4.c
 * \brief           hosal_crytpo_sm4 file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mcu.h"
#include "hosal_crypto_sm4.h"

int hosal_crypto_init(void) {

    int status = STATUS_SUCCESS;

    crypto_lib_init();

    return status;
}

int hosal_crypto_sm4_operation(hosal_sm4_dev_t* sm4_dev) {
    int status, i;

    status = STATUS_SUCCESS;

    sm4_acquire();

    if (sm4_dev->crypto_operation == HOSAL_SM4_ENCODE) {

        for (i = 0; i < sm4_dev->loop; i++) {

            sm4_ecb_encode(sm4_dev->out_ptr, sm4_dev->in_ptr, sm4_dev->mkey_ptr);
        }


    } else if (sm4_dev->crypto_operation == HOSAL_SM4_DECODE) {

        for (i = 0; i < sm4_dev->loop; i++) {

            sm4_ecb_decode(sm4_dev->out_ptr, sm4_dev->in_ptr, sm4_dev->mkey_ptr);
        }

    } else {
        status = STATUS_INVALID_PARAM;
    }

    sm4_release();

    return status;
}






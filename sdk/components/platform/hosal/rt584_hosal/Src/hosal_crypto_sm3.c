/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_sm3.c
 * \brief           hosal_crytpo_sm3 file
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
#include "rt_sm3.h"
#include "hosal_crypto_sm3.h"


int hosal_crypto_sm3_operation(hosal_sm3_dev_t* sm3_dev) {
    int status;

    status = STATUS_SUCCESS;

    if (sm3_dev->crypto_operation == HOSAL_SM3_DIGEST) {
        sm3_init(&sm3_dev->sm3_cnxt);
        sm3_update(&sm3_dev->sm3_cnxt, sm3_dev->in_ptr, sm3_dev->in_length);
        sm3_final(&sm3_dev->sm3_cnxt, sm3_dev->out_ptr);
    } else if (sm3_dev->crypto_operation == HOSAL_SM3_INIT) {
        sm3_init(&sm3_dev->sm3_cnxt);
    } else if (sm3_dev->crypto_operation == HOSAL_SM3_UPDATE) {
        sm3_update(&sm3_dev->sm3_cnxt, sm3_dev->in_ptr, sm3_dev->in_length);
    } else if (sm3_dev->crypto_operation == HOSAL_SM3_FINAL) {
        sm3_final(&sm3_dev->sm3_cnxt, sm3_dev->out_ptr);
    } else {
        return STATUS_INVALID_PARAM;
    }

    return status;
}





/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_trng.c
 * \brief           Hosal TRNG driver file
 */
/*
 * Author:       
 */

#include "mcu.h"
#include "hosal_trng.h"
#include "hosal_status.h"
#include "trng.h"

int hosal_trng_get_random_number(uint32_t *buffer, uint32_t number) {

    get_random_number(buffer,number);

    return HOSAL_STATUS_SUCCESS;
}







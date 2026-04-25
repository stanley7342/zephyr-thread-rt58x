/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            trng.c
 * \brief           trng driver file
 */
/*
 * Author:          ives.lee
 */


#include "trng.h"
#include "sysctrl.h"
#include "pufs_rt_regs.h"

uint32_t get_random_number(uint32_t *p_buffer, uint32_t number)
{
    uint32_t  i, status = STATUS_SUCCESS;

    enter_critical_section();

    enable_secure_write_protect();

    status = rt_write_rng_enable(true);

    if (status == STATUS_SUCCESS)
    {
        for (i = 0; i < number ; ++i)
        {
            p_buffer[i] = OTP_RNG_S->data;
        }
    }

    rt_write_rng_enable(false);

    disable_secure_write_protect();

    leave_critical_section();

    return status;
}


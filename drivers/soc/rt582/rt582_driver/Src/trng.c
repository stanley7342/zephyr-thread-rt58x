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

int get_random_number(uint32_t* p_buffer, uint32_t number) {
    /*
     * Please note this function is block mode. it will block about 4 ms ~ 6 ms
     * If you don't want the code block here, use interrupt mode.
     */
    uint32_t temp,i;
    volatile uint32_t *ptr;

    for(i=0;i<number;i++)
    {
        SYSCTRL->trng1 = 0x0; /*select von Neumann */

        /*TRNG*/
        SYSCTRL->trng0 = 1;

        while (SYSCTRL->trng2 & 0x1)
            ;

        /*Clear interrut pending*/
        SYSCTRL->trng0 = 0x2;
        /*Get random number */
        ptr = (volatile uint32_t *) p_buffer;

        *ptr++ = SYSCTRL->trng3;
    }

    return STATUS_SUCCESS;
}


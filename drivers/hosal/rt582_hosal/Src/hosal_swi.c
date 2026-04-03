/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_swi.c
 * \brief           hosal software interrupt driver file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#include <stdint.h>
#include "stdio.h"
#include "mcu.h"
#include "hosal_swi.h"
#include "hosal_status.h"
#include "swi.h"


int hosal_swi_uninit(void) {
    uint32_t  i,j;
    
    for (i = 0; i < HOSAL_SWI_ID_MAX; i++) {

        for(j=0;j<HOSAL_MAX_NUMBER_OF_SWI;j++) {

            swi_int_disable(i, j);
        }
    }

    return HOSAL_STATUS_SUCCESS;
}


int hosal_swi_init(void) {
    
    uint32_t  i;

     for (i = 0; i < HOSAL_SWI_ID_MAX; i++) {

         swi_int_callback_clear(i);
    }   
    
    return HOSAL_STATUS_SUCCESS;
}

int hosal_swi_trigger(uint32_t swi_port, uint32_t swi_trig_bit) {

    if (swi_port > HOSAL_SWI_ID_MAX ||swi_trig_bit==HOSAL_MAX_NUMBER_OF_SWI)  {

        return HOSAL_STATUS_INVALID_PARAM;
    }

    swi_int_trigger(swi_port,swi_trig_bit);

    return HOSAL_STATUS_SUCCESS;
}


int hosal_swi_callback_register(uint32_t swi_port, uint32_t swi_trig_bit, hosal_swi_cb cb) {

      if (swi_port > HOSAL_SWI_ID_MAX || swi_trig_bit==HOSAL_MAX_NUMBER_OF_SWI)  {

        return HOSAL_STATUS_INVALID_PARAM;
    }

	swi_int_enable(swi_port,swi_trig_bit,cb);
		
    return HOSAL_STATUS_SUCCESS;
}

int hosal_swi_callback_unregister(uint32_t swi_port, uint32_t swi_trig_bit) {

     if (swi_port > HOSAL_SWI_ID_MAX || swi_trig_bit==HOSAL_MAX_NUMBER_OF_SWI) {

        return HOSAL_STATUS_INVALID_PARAM;
    }

    NVIC_DisableIRQ((IRQn_Type)(Soft_IRQn + swi_port));

    swi_int_disable(swi_port,swi_trig_bit);

    return HOSAL_STATUS_SUCCESS;
}


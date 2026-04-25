/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**
 * \file            hosal_dma.c
  * \brief          Hosal Dma driver file
 */

/*
 * This file is part of library_name.
 * Author:   
 */

#include "stdio.h"
#include <stdint.h>
#include "mcu.h"
#include "dma.h"
#include "hosal_dma.h"
#include "hosal_status.h"

int hosal_dma_unint(void) {
    //dma_uninit(); rt58x(1/2/3) only
    return HOSAL_STATUS_SUCCESS;
}


int hosal_dma_init(void) {
    //dma_init(); rt58x(1/2/3) only
    return HOSAL_STATUS_SUCCESS;
}


int hosal_dma_interrupt_mode(hosal_dma_dev_t *dma_dev) {
   
     dma_config_t   dma_cfg;

    if((dma_dev->callbackfn==NULL) || (dma_dev->channel >= HOSAL_DMA_ID_MAX))
    {
        return HOSAL_STATUS_INVALID_PARAM;
    }

     dma_cfg.dma_src_addr =  dma_dev->src_address;
     dma_cfg.dma_dest_addr = dma_dev->dst_address;
     dma_cfg.dma_length = dma_dev->size;


     dma_register_isr(dma_dev->channel, dma_dev->callbackfn);
     dma_config_and_enable(dma_dev->channel, &dma_cfg);
  
    return HOSAL_STATUS_SUCCESS;
}


int hosal_dma_polling_mode(hosal_dma_dev_t *dma_dev) {

    dma_config_t  dma_cfg;
    
    dma_cfg.dma_src_addr =  dma_dev->src_address;
    dma_cfg.dma_dest_addr = dma_dev->dst_address;
    dma_cfg.dma_length = dma_dev->size;

    dma_config_and_enable_polling(dma_dev->channel, &dma_cfg);

    return HOSAL_STATUS_SUCCESS;
}






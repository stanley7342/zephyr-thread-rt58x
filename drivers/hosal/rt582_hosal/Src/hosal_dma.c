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

#include <stdint.h>
#include "mcu.h"
#include "stdio.h"

#include "dma.h"
#include "hosal_dma.h"
#include "hosal_status.h"

int hosal_dma_unint(void) {
    dma_uninit();
    return HOSAL_STATUS_SUCCESS;
}

int hosal_dma_init(void) {
    dma_init();
    return HOSAL_STATUS_SUCCESS;
}

int hosal_dma_interrupt_mode(hosal_dma_dev_t* dma_dev) {
    dma_config_t dma_mode_config = {
        .dma_control_setting = DMA_CNTL_SRC_INC | DMA_CNTL_SRC_WIDTH_32BIT
                               | DMA_CNTL_SRC_BURST4 | DMA_CNTL_SRC_PORT
                               | DMA_CNTL_DEST_INC | DMA_CNTL_DEST_WIDTH_32BIT
                               | DMA_CNTL_DEST_BURST4 | DMA_CNTL_DEST_PORT
                               | DMA_CNTL_MODE_MEMTOMEM,
        .dma_src_addr = dma_dev->src_address,
        .dma_dest_addr = dma_dev->dst_address,
        .dma_xfer_length = dma_dev->size,
        /* We don't use DMA repeat mode in memory to memory */
        .dma_repeat_setting = 0,
        /* We use interrupt to notify Finish */
        .dma_int_enable = DMA_INT_ALL,
    };

    dma_register_isr(dma_dev->channel, dma_dev->callbackfn);
    dma_config(dma_dev->channel, &dma_mode_config);
    dma_enable(dma_dev->channel);
    return HOSAL_STATUS_SUCCESS;
}

int hosal_dma_polling_mode(hosal_dma_dev_t* dma_dev) {
    dma_config_t dma_mode_config = {
        .dma_control_setting = DMA_CNTL_SRC_INC | DMA_CNTL_SRC_WIDTH_32BIT
                               | DMA_CNTL_SRC_BURST4 | DMA_CNTL_SRC_PORT
                               | DMA_CNTL_DEST_INC | DMA_CNTL_DEST_WIDTH_32BIT
                               | DMA_CNTL_DEST_BURST4 | DMA_CNTL_DEST_PORT
                               | DMA_CNTL_MODE_MEMTOMEM,
        .dma_src_addr = (uint32_t)dma_dev->src_address,
        .dma_dest_addr = (uint32_t)dma_dev->dst_address,
        .dma_xfer_length = dma_dev->size,
        /* We don't use DMA repeat mode in memory to memory */
        .dma_repeat_setting = 0,
        /* Polled mode, not enable interrupt */
        .dma_int_enable = 0,
    };

    dma_config(dma_dev->channel, &dma_mode_config);
    dma_enable(dma_dev->channel);

    uint32_t dma_status = 0;
    do {
        dma_get_status(dma_dev->channel, &dma_status);
    } while ((dma_status & DMA_INT_XFER_DONE) != DMA_INT_XFER_DONE);

    dma_finish(dma_dev->channel);
    return HOSAL_STATUS_SUCCESS;
}

int hosal_dma_linklist(hosal_dma_dev_t* dma_dev,
                       hosal_dma_link_dev_t* dev_link_dev) {
    /*the following setting is linkListEnable, meory to memory. */
    uint32_t dma_control_mode = DMA_CNTL_SRC_WIDTH_32BIT
                                | DMA_CNTL_DEST_WIDTH_32BIT | DMA_CNTL_SRC_INC
                                | DMA_CNTL_SRC_BURST8 | DMA_CNTL_SRC_PORT
                                | DMA_CNTL_DEST_INC | DMA_CNTL_DEST_BURST8
                                | DMA_CNTL_DEST_PORT | DMA_CNTL_MODE_MEMTOMEM
                                | DMA_CNTL_LINKLIST_ENABLE;

    /*create a LinkList reqest for hardware..*/
    create_dma_link_list(
        (dma_linklist_request_t*)(dev_link_dev->hosal_dma_link_request),
        dev_link_dev->hosal_item_number,
        (dma_linklist_entry_t*)(dev_link_dev->hosal_dma_link_list_entry),
        dma_control_mode);

    /*set dma to link list mode*/
    dma_set_link_list(
        dma_dev->channel,
        (dma_linklist_entry_t*)(dev_link_dev->hosal_dma_link_list_entry),
        (DMA_INT_LL_ELEMENT_Done | DMA_INT_XFER_DONE));

    /*start DMA */
    dma_enable(dma_dev->channel);
    return HOSAL_STATUS_SUCCESS;
}

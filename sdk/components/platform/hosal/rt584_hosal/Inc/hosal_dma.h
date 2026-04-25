/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_dam.h
 * \brief           hosal_dma include file
 */
/*
 * This file is part of library_name.
 * Author:  ives.lee 
 */

#ifndef HOSAL_DMA_H
#define HOSAL_DMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "mcu.h"

/**
 * \defgroup        HOSAL_DMA Hosal dma
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal dma definitions, structures, and functions
 * @{
 */

typedef void (*hosal_dma_cb_fn)(uint32_t channel_id,uint32_t status);

/**
 * \brief           DMA id enum struct.
 */
typedef enum {
    HOSAL_DMA_ID_0 = 0,                         /*!< dma id 0 */ 
    HOSAL_DMA_ID_1 = 1,                         /*!< dma id 1 */ 
    HOSAL_DMA_ID_MAX=2                          /*!< dma max id */ 
} hosal_dma_id_t;

/**
 * \brief           DMA device control setting struct.
 */
typedef struct {
    uint8_t         channel;                    /*!< dma device channel */ 
    uint32_t        src_address;                /*!< dma source address */ 
    uint32_t        dst_address;                /*!< dma destination address */ 
    uint32_t        size;                       /*!< dma size */ 
    hosal_dma_cb_fn callbackfn;                 /*!< dma callback function */ 
} hosal_dma_dev_t;

/**
 * \brief           Dma uninit function
 * \param[in]       NONE
 * \return          function status
 */
int hosal_dma_unint(void);

/**
 * \brief           Dma init function
 * \param[in]       NONE
 * \return          function status
 */
int hosal_dma_init(void);

/**
 * \brief           Dma interrupt mode function
 * \param[in]       dma_dev: 
 * \return          function status 
 */
int hosal_dma_interrupt_mode(hosal_dma_dev_t *dma_dev);

/**
 * \brief           Dma polling mode function
 * \param           dma_dev: 
 * \return          function status 
 */
int hosal_dma_polling_mode(hosal_dma_dev_t *dma_dev);

/*@}*/ /* end of RT584_HOSAL HOSAL_DMA */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_DMA_H */

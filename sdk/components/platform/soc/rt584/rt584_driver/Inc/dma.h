/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dma.h
 * \brief           dma header file
 */
/*
 * This file is part of library_name.
 * Author: ives.lee
 */


#ifndef DMA_H
#define DMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"


/**
 * \defgroup        DMA Dma
 * \ingroup         RT584_DRIVER
 * \brief           Define Dma comp definitions, structures, and functions
 * @{
 */

/**
 * \brief 
 */
#define MAX_NUMBER_OF_DMA      2
#define DMA_STATUS_BUSY        1
#define DMA_STATUS_FREE        0


/**
 * \brief           DMA finish routine callback for user application.
 * \param[in]       chennel_id: DMA channel_id
 * \param[in]       statue: DMA transfer status
 */
typedef void (*dma_proc_cb_t)(uint32_t channel_id,uint32_t status);

/**
 * \brief           DMA ID definitions
 */
typedef enum {
    DMA_ID_0 = 0,                               /*!dma 0 identifie */
    DMA_ID_1 = 1,                               /*!dma 1 identifie */
    DMA_ID_MAX = 2,
} dma_id_t;

/**
 * \brief           DMA config structure for DMA setting
 */
typedef struct {
    uint32_t   dma_src_addr;                    /*!dma source address */
    uint32_t   dma_dest_addr;                   /*!dma destination addreess */
    uint32_t   dma_length;                      /*!dma length */
} dma_config_t;


/**
 * \brief           DMA interrupt ISR function register. Use to notify channel DMA finished.
 * \param[in]       dma_channel_id: Specifies the dma channel number.
 * \param[in]       app_dma_callback:
 */
uint32_t dma_register_isr(uint32_t  dma_channel_id, dma_proc_cb_t  app_dma_callback);

/**
 * \brief           DMA config. Use to config DMA mode and start the DMA.
 * \param[in]       dma_channel_id: Specifies the dma channel number.
 * \param[in]       *dma_config_t: Specifies the dma setting.
 */

uint32_t dma_config_and_enable(uint32_t  dma_channel_id, const dma_config_t *mode);

/**
 * \brief           DMA config. Use to config DMA mode and start the DMA. wait dma bush flag finish
 * \param[in] dma_channel_id   Specifies the dma channel number.
 * \param[in] *dma_config_t    Specifies the dma setting.
 */

uint32_t dma_config_and_enable_polling(uint32_t  dma_channel_id, const dma_config_t *mode);

/**
 * \brief           Get DMA transfer status.
 * \param[in]       dma_channel_id: Specifies the dma channel number.
 * \param[in]       status: DMA tranfer status.
 */
uint32_t dma_get_status(uint32_t  dma_channel_id, uint32_t *status);

/*@}*/ /* end of RT584_DRIVER DMA */

#ifdef __cplusplus
}
#endif

#endif /* End of DMA_H */

/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dma_reg.h
 * \brief           dma register header file
 */
/*
 * This file is part of library_name.
 * Author: ives.lee
 */


#ifndef DMA_REG_H
#define DMA_REG_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief          Dma total register 
 */
typedef struct {
    __IO uint32_t dma_src_adr;                 /*!< offset:0x00 */
    __IO uint32_t dma_dest_adr;                /*!< offset:0x04 */
    __IO uint32_t dma_bytes_num;               /*!< offset:0x08 */
    __IO uint32_t dma_control;                 /*!< offset:0x0c */
    __IO uint32_t dma_int;                     /*!< offset:0x10 */
    __IO uint32_t dma_port;                    /*!< offset:0x14 */
} dma_t;


#define DMA_START_ENABLE                (1<<0)
#define DMA_BUSY                        (1<<8)

#define DMA_CHL_INT_STATUS              (1<<0)
#define DMA_CHL_WAKEUP_STATUS           (1<<1)

#define DMA_CHL_INT_ENABLE              (1<<8)
#define DMA_CHL_INT_WAKEUP              (1<<9)

#define DMA_CHL_INT_CLEAR_CLR           (1<<16)
#define DMA_CHL_INT_WAKEUP_CLR          (1<<17)

#ifdef __cplusplus
}
#endif

#endif /* End of DMA_REG_H */

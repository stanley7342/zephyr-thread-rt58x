/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dam_reg.h
 * \brief           dam_reg.h include file
 */

/*
 * This file is part of library_name.
 * Author:
 */

#ifndef DMA_REG_H
#define DMA_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DMA_STATUS_ENABLE (1 << 7)
#define DMA_DMACLK_GATE   (1 << 1)
#define DMA_ENABLE_MASK   (1 << 0)
#define DMA_ENABLE_BIT    (1 << 0)

/**
 * \brief           This is point struct
 * \note            This structure is used to calculate all point
 *                      related stuff
 */
typedef struct {
    __IO uint32_t dma_ctrl;                     /*!< offset 0x00 dma control register */
    __I uint32_t dma_status;                    /*!< offset 0x04 dma statsu register */
    __IO uint32_t dma_source_desc;              /*!< offset 0x08 dma soucre address */
    __IO uint32_t dma_dest_desc;                /*!< offset 0x0c dma destions address */
    __IO uint32_t dma_xfer_len;                 /*!< offset 0x10 dam xfer length */
    __I uint32_t dma_cur_src_addr;              /*!< offset 0x14 dma current source address */
    __I uint32_t dma_cur_dest_addr;             /*!< offset 0x18 dma current destions address */
    __IO uint32_t dma_intr_ctrl;                /*!< offset 0x1c dma interrupt control register */
    __O uint32_t dma_intr_clear;                /*!< offset 0x20 dam interrupt control clear register */
    __IO uint32_t dma_enable;                   /*!< offset 0x24 dma enable register */
    __I uint32_t dma_lli_next_ptr;              /*!< offset 0x28 dma link list next point register  */
    __IO uint32_t dma_repeat_value;             /*!< offset 0x2c dam repeat value register */
    __I uint32_t dma_repeat_status;             /*!< offset 0x30 dma repeat status register */
} dma_t;

#ifdef __cplusplus
}
#endif

#endif /* End of DMA_REG_H */

/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            pwm.h
 * \brief           pwm header file
 */
/*
 * This file is part of library_name.
 * Author:
 */

#ifndef QSPI_REG_H
#define QSPI_REG_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           qspi tx/rx fifo offset
 */
#define QSPI_TX_FIFO_OFFSET 0
#define QSPI_RX_FIFO_OFFSET 4

#define QSPI_CNTL_NODMA_MASK (0xFF)
/**
 * \brief           qspi data bit size shift
 */
#define QSPI_BITSIZE_8  (1 << 4)
#define QSPI_BITSIZE_32 (7 << 4)
/**
 * \brief           qspi tx/rx input/output enable
 */
#define QSPI_DISABLE_OUT (1 << 2)
#define QSPI_DISABLE_IN  (1 << 3)
/**
 * \brief           qspi const define
 */
#define QSPI_Xfer_Extend (1 << 7)

#define QSPI_CNTL_contXfer_SHIFT (0)
#define QSPI_CNTL_contXfer_En    (1 << QSPI_CNTL_contXfer_SHIFT)
/**
 * \brief           qspi data little endian value
 */
#define QSPI_CNTL_EDIAN_SHIFT   (1)
#define QSPI_CNTL_LITTLE_ENDIAN (1 << QSPI_CNTL_EDIAN_SHIFT)
/**
 * \brief           qspi bit order shift value
 */
#define QSPI_CNTL_MSB_SHIFT  (2)
/**
 * \brief           qspi phase and politary shift value
 */
#define QSPI_CNTL_CPHA_SHIFT (3)
#define QSPI_CNTL_CPOL_SHIFT (4)
/**
 * \brief           qspi master mode shift and enable
 */
#define QSPI_CNTL_MASTER_SHIFT (5)
#define QSPI_CNTL_MASTER       (1 << 5)
#define QSPI_CNTL_SLAVE        (0 << 5)
/**
 * \brief           qspi slave mode shift and enable
 */
#define SPI_CNTL_SLAVE_SDATA_SHIFT (6)
#define SPI_CNTL_SLAVE_SDATA_OUT   (1 << 6)
/**
 * \brief           qspi dma  contrl enable shift
 */
#define QSPI_CNTL_DMA_SHIFT (10)
/**
 * \brief           qspi mwait delay shift
 */
#define QSPI_CNTL_mWaitEn_SHIFT (11)
/**
 * \brief           tx/rx water mask shift
 */
#define QSPI_CNTL_rxWmark_SHIFT (12)
#define QSPI_CNTL_txWmark_SHIFT (14)

/**
 * \brief           qspi dma contrl enable value
 */
#define QSPI_CNTL_DMA_ENABLE (1 << QSPI_CNTL_DMA_SHIFT)
/**
 * \brief           qspi  tx watermark
 */
#define QSPI_CNTL_TX_1_8_WATERMARK  (00 << QSPI_CNTL_txWmark_SHIFT)
#define QSPI_CNTL_TX_1_4_WATERMARK  (01 << QSPI_CNTL_txWmark_SHIFT)
#define QSPI_CNTL_TX_HALF_WATERMARK (10 << QSPI_CNTL_txWmark_SHIFT)
#define QSPI_CNTL_TX_3_4_WATERMARK  (11 << QSPI_CNTL_txWmark_SHIFT)
/**
 * \brief           qspi  rx watermark
 */
#define QSPI_CNTL_RX_1_8_WATERMARK  (00 << QSPI_CNTL_rxWmark_SHIFT)
#define QSPI_CNTL_RX_1_4_WATERMARK  (01 << QSPI_CNTL_rxWmark_SHIFT)
#define QSPI_CNTL_RX_HALF_WATERMARK (10 << QSPI_CNTL_rxWmark_SHIFT)
#define QSPI_CNTL_RX_3_4_WATERMARK  (11 << QSPI_CNTL_rxWmark_SHIFT)
/**
 * \brief           qspi status const 
 */
#define QSPI_STATUS_xferIP     (1 << 0) /*transfer in progress*/
#define QSPI_STATUS_AllCmdDone (1 << 1)
#define QSPI_STATUS_txEmpty    (1 << 2)
#define QSPI_STATUS_txWmark    (1 << 3)
#define QSPI_STATUS_txFull     (1 << 4)
#define QSPI_STATUS_rxEmpty    (1 << 5)
#define QSPI_STATUS_rxWmark    (1 << 6)
#define QSPI_STATUS_rxFull     (1 << 7)
/**
 * \brief           qspi interrupt mask 
 */
#define QSPI_INT_txEmpty    (1 << 0)
#define QSPI_INT_txWmark    (1 << 1)
#define QSPI_INT_rxWmark    (1 << 2)
#define QSPI_INT_rxFull     (1 << 3)
#define QSPI_INT_xferDone   (1 << 4)
#define QSPI_INT_rxNotEmpty (1 << 5)
/**
 * \brief           qspi clock output div value
 */
#define QSPI_MST_CLKDIV_EN (1 << 8)

#define QSPI_MST_CLKDIV_16MHZ (0)
#define QSPI_MST_CLKDIV_8MHZ  (1)
#define QSPI_MST_CLKDIV_4MHZ  (3)
#define QSPI_MST_CLKDIV_2MHZ  (7)
#define QSPI_MST_CLKDIV_1MHZ  (15)
/**
 * \brief           qspi dma interrupt and enable const definetions
 */
#define QSPI_DMA_ISR_TX (1 << 1)
#define QSPI_DMA_ISR_RX (1 << 0)
#define QSPI_DMA_ISR_CLEARALL (QSPI_DMA_ISR_TX | QSPI_DMA_ISR_RX)
#define QSPI_DMA_IER_TX QSPI_DMA_ISR_TX
#define QSPI_DMA_IER_RX QSPI_DMA_ISR_RX
#define QSPI_DMA_ENABLE       (1 << 0)
#define QSPI_DMA_Dummy_ENABLE (1 << 1)

#define SPI0_CLK_INDEX (20)

typedef struct {
    __IO uint32_t qspi_tx_fifo;         //0x0
    __I uint32_t qspi_rx_fifo;          //0x4
    __I uint32_t reserve_1;             //0x8
    __IO uint32_t qspi_control;         //0xc
    __IO uint32_t qspi_aux;             //0x10
    __I uint32_t qspi_status;           //0x14
    __IO uint32_t qspi_slave_sel;       //0x18
    __IO uint32_t qspi_slave_sel_pol;   //0x1c
    __IO uint32_t qspi_int_en;          //0x20
    __I uint32_t qspi_int_status;       //0x24
    __IO int32_t qspi_int_clr;          //0x28
    __I uint32_t qspi_tx_fifo_lvl;      //0x2c
    __I uint32_t qspi_rx_fifo_lvl;      //0x30
    __I uint32_t reserve_2;             //0x34
    __IO uint32_t qspi_m_wait;          //0x38
    __IO uint32_t qspi_en;              //0x3c
    __IO uint32_t reserved[4];          //0x40~0x4c
    __IO uint32_t qspi_clkdiv;          //0x50
    __IO uint32_t reserved2[3];         //0x54~0x5c
    __IO uint32_t dma_rx_addr;          //0x60
    __IO uint32_t dma_rx_len;           //0x64
    __IO uint32_t dma_tx_addr;          //0x68
    __IO uint32_t dma_tx_len;           //0x6c
    __I uint32_t dma_rx_rlen;           //0x70
    __I uint32_t dma_tx_rlen;           //0x74
    __IO uint32_t dma_ier;              //0x78
    __IO uint32_t dma_int_status;       //0x7c
    __IO uint32_t dma_rx_enable;        //0x80
    __IO uint32_t dma_tx_enable;        //0x84
} qspi_t;




#ifdef __cplusplus
}
#endif

#endif /* End of QSPI_REG_H */

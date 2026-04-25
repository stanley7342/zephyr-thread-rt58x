/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           qspi_reg.h
 * \brief          qspi register header file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef QSPI_MPA_REG_H
#define QSPI_MPA_REG_H

#include "mcu.h"

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif


typedef union
{
    struct
    {
        uint32_t cfg_short_cycle_en: 1;
        uint32_t rvd0: 7;
        uint32_t cfg_short_cycle_num: 5;
        uint32_t rvd1: 3;
        uint32_t cfg_direct_bit_en: 1;
        uint32_t rvd2: 7;
        uint32_t cfg_direct_bit_num: 5;
        uint32_t rvd3: 3;
    } bit;
    uint32_t Reg;
} edp_func_t;

typedef struct
{
    __IO  uint32_t   qspi_tx_fifo;           //0x0
    __I   uint32_t   qspi_rx_fifo;           //0x4
    __IO  uint32_t   qspi_control;           //0x8
    __IO  uint32_t   qspi_control2;          //0xC
    __IO  uint32_t   qspi_ss_config;         //0x10
    __IO  uint32_t   qspi_clkdiv;            //0x14
    __IO  edp_func_t qspi_epd_func;          //0x18
    __IO  uint32_t   qspi_delays;            //0x1C

    __IO  uint32_t   qspi_int_en;            //0x20
    __I   uint32_t   qspi_int_status;        //0x24
    __IO  int32_t    qspi_int_clr;           //0x28

    __IO  uint32_t   qspi_en;                //0x2C


    __I   uint32_t   qspi_status;            //0x30
    __I   uint32_t   qspi_tx_fifo_lvl;       //0x34
    __I   uint32_t   qspi_rx_fifo_lvl;       //0x38
    __I   uint32_t   reserve_1;              //0x3C

    __IO  uint32_t   dma_rx_addr;            //0x40
    __IO  uint32_t   dma_rx_len;             //0x44
    __IO  uint32_t   dma_tx_addr;            //0x48
    __IO  uint32_t   dma_tx_len;             //0x4C

    __I   uint32_t   dma_rx_rlen;            //0x50
    __I   uint32_t   dma_tx_rlen;            //0x54
    __IO  uint32_t   dma_ier;                //0x58
    __IO  uint32_t   dma_int_status;         //0x5C

    __IO  uint32_t   dma_rx_enable;          //0x60
    __IO  uint32_t   dma_tx_enable;          //0x64

} qspi_t;

#define  QSPI_TX_FIFO_OFFSET          0
#define  QSPI_RX_FIFO_OFFSET          4

/*QSPI_CONTROL */
#define  QSPI_CNTL_MASTER_SHIFT      (0)
#define  QSPI_CNTL_MASTER            (1<<QSPI_CNTL_MASTER_SHIFT)
#define  QSPI_CNTL_SLAVE             (0<<QSPI_CNTL_MASTER_SHIFT)

#define  QSPI_CNTL_CPHA_SHIFT        (1)
#define  QSPI_CNTL_CPOL_SHIFT        (2)

#define  SPI_CNTL_SLAVE_SDATA_SHIFT  (3)
#define  SPI_CNTL_SLAVE_SDATA_OUT    (1<<3)

#define  QSPI_CNTL_EDIAN_SHIFT       (4)
#define  QSPI_CNTL_LITTLE_ENDIAN     (1<<QSPI_CNTL_EDIAN_SHIFT)

#define  QSPI_CNTL_MSB_SHIFT         (5)
#define  QSPI_CNTL_MSB_FIRST         (1<<QSPI_CNTL_MSB_SHIFT)
#define  QSPI_CNTL_LSB_FIRST         (0<<QSPI_CNTL_MSB_SHIFT)

#define  QSPI_CNTL_contXfer_SHIFT    (6)
#define  QSPI_CNTL_contXfer_En       (1<<QSPI_CNTL_contXfer_SHIFT)


#define  QSPI_CNTL_PREDELAY_SHIFT        (8)
#define  QSPI_CNTL_PREDELAY_ENABLE       (1<<8)
#define  QSPI_CNTL_PREDELAY_DISABLE      (0<<8)

#define  QSPI_CNTL_INTER_DELAY_SHIFT     (9)
#define  QSPI_CNTL_INTER_DELAY_ENEABLE   (1<<9)
#define  QSPI_CNTL_INTER_DELAY_DISABLE   (0<<9)

#define  QSPI_CNTL_POSTDELAY_SHIFT       (10)
#define  QSPI_CNTL_POSTDELAY_ENABLE      (1<<10)
#define  QSPI_CNTL_POSTDELAY_DISABLE     (0<<10)


#define  QSPI_CNTL_rxWmark_SHIFT        (12)
#define  QSPI_CNTL_txWmark_SHIFT        (14)

#define  QSPI_CNTL_TX_1_8_WATERMARK     (0<<QSPI_CNTL_txWmark_SHIFT)
#define  QSPI_CNTL_TX_1_4_WATERMARK     (1<<QSPI_CNTL_txWmark_SHIFT)
#define  QSPI_CNTL_TX_HALF_WATERMARK    (2<<QSPI_CNTL_txWmark_SHIFT)
#define  QSPI_CNTL_TX_3_4_WATERMARK     (3<<QSPI_CNTL_txWmark_SHIFT)

#define  QSPI_CNTL_RX_1_8_WATERMARK     (0<<QSPI_CNTL_rxWmark_SHIFT)
#define  QSPI_CNTL_RX_1_4_WATERMARK     (1<<QSPI_CNTL_rxWmark_SHIFT)
#define  QSPI_CNTL_RX_HALF_WATERMARK    (2<<QSPI_CNTL_rxWmark_SHIFT)
#define  QSPI_CNTL_RX_3_4_WATERMARK     (3<<QSPI_CNTL_rxWmark_SHIFT)


#define  QSPI_CNTL_TX_4BYTE_WATERMARK       (0<<QSPI_CNTL_txWmark_SHIFT)
#define  QSPI_CNTL_TX_8BYTE_WATERMARK       (1<<QSPI_CNTL_txWmark_SHIFT)
#define  QSPI_CNTL_TX_16BYTE_WATERMARK      (2<<QSPI_CNTL_txWmark_SHIFT)
#define  QSPI_CNTL_TX_20BYTE_WATERMARK      (3<<QSPI_CNTL_txWmark_SHIFT)

#define  QSPI_CNTL_RX_4BYTE_WATERMARK       (0<<QSPI_CNTL_rxWmark_SHIFT)
#define  QSPI_CNTL_RX_8BYTE_WATERMARK       (1<<QSPI_CNTL_rxWmark_SHIFT)
#define  QSPI_CNTL_RX_16BYTE_WATERMARK      (2<<QSPI_CNTL_rxWmark_SHIFT)
#define  QSPI_CNTL_RX_20BYTE_WATERMARK      (3<<QSPI_CNTL_rxWmark_SHIFT)

#define  QSPI_WIRE_MODE_SHIFT      (0)

#define  QSPI_BITSIZE_8            (1<<4)
#define  QSPI_BITSIZE_16           (3<<4)
#define  QSPI_BITSIZE_32           (7<<4)

#define  QSPI_DISABLE_OUT          (1<<2)
#define  QSPI_DISABLE_IN           (1<<3)

#define  QSPI_Xfer_Extend          (1<<7)


#define  QSPI_PRE_DELAY_SHIFT            (0)
#define  QSPI_INTER_DELAY_SHIFT          (8)
#define  QSPI_POST_DELAY_SHIFT           (16)

#define  QSPI_PRE_DELAY_MASK             (0xFF << QSPI_PRE_DELAY_SHIFT)
#define  QSPI_INTER_DELAY_MASK           (0xFF << QSPI_INTER_DELAY_SHIFT)
#define  QSPI_POST_DELAY_MASK            (0xFF << QSPI_POST_DELAY_SHIFT)

#define  QSPI_SSOUT_SHIFT                (0)

#define  QSPI_SSOUT_MASK                 (0xF<<QSPI_SSOUT_SHIFT)

#define  QSPI_CFG_SSPOL_SHIFT            (8)

#define  QSPI_CFG_SSPOL_MASK             (0xF<<QSPI_CFG_SSPOL_SHIFT)

#define  QSPI_CFG_SS_MANUAL_SHIFT            (16)
#define  QSPI_CFG_SS_MANUUAL_ENABLE_SHIFT    (24)


#define  QSPI_MST_CLKDIV_EN           (1<<8)

/*the following setting base 32MHz OSC*/
#define  QSPI_MST_CLKDIV_16MHZ        (0)
#define  QSPI_MST_CLKDIV_8MHZ         (1)
#define  QSPI_MST_CLKDIV_4MHZ         (3)
#define  QSPI_MST_CLKDIV_2MHZ         (7)
#define  QSPI_MST_CLKDIV_1MHZ         (15)


#define  QSPI_INT_txEmpty             (1<<0)
#define  QSPI_INT_under_txWmark       (1<<1)
#define  QSPI_INT_over_rxWmark        (1<<2)
#define  QSPI_INT_rxFull              (1<<3)
#define  QSPI_INT_xferDone            (1<<4)
#define  QSPI_INT_rxNotEmpty          (1<<5)

#define  QSPI_STATUS_xferIP           (1<<0)            /*transfer in progress*/
#define  QSPI_STATUS_AllCmdDone       (1<<1)
#define  QSPI_STATUS_txEmpty          (1<<2)
#define  QSPI_STATUS_txWmark          (1<<3)
#define  QSPI_STATUS_txFull           (1<<4)
#define  QSPI_STATUS_rxEmpty          (1<<5)
#define  QSPI_STATUS_rxWmark          (1<<6)
#define  QSPI_STATUS_rxFull           (1<<7)


#define  QSPI_MST_CLKDIV_EN           (1<<8)

#define  QSPI_MST_CLKDIV_16MHZ        (0)
#define  QSPI_MST_CLKDIV_8MHZ         (1)
#define  QSPI_MST_CLKDIV_4MHZ         (3)
#define  QSPI_MST_CLKDIV_2MHZ         (7)
#define  QSPI_MST_CLKDIV_1MHZ         (15)


#define  QSPI_DMA_ISR_TX              (1<<1)
#define  QSPI_DMA_ISR_RX              (1<<0)

#define  QSPI_DMA_ISR_CLEARALL        (QSPI_DMA_ISR_TX|QSPI_DMA_ISR_RX)


#define  QSPI_DMA_IER_TX              QSPI_DMA_ISR_TX
#define  QSPI_DMA_IER_RX              QSPI_DMA_ISR_RX

#define  QSPI_DMA_ENABLE              (1<<0)
#define  QSPI_DMA_Dummy_ENABLE        (1<<1)


#define  SPI0_CLK_INDEX               (20)




#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif

#endif

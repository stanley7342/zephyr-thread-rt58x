/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2s_reg.h
 * \brief           i2s register header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef I2S_REG_H
#define I2S_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           I2S control0 register at offet 0x00
 */
typedef union i2s_ctrl0_s {
    struct i2s_ctrl0_b {
        uint32_t i2s_enable  : 1;               /*!< i2s enable bit */
        uint32_t mclk_enable : 1;               /*!< mclk enable bits */
        uint32_t reserved    : 30;              /*!< reserved bits */
    } bit;

    uint32_t reg;
} i2s_ctrl0_t;

/**
 * \brief           I2S control1 register at offet 0x04
 */
typedef union i2s_ctrl1_s {
    struct i2s_ctrl1_b {
        uint32_t i2s_reset : 1;                 /*!< i2s reset  bit */
        uint32_t reserved  : 31;                /*!< reserved bits */
    } bit;

    uint32_t reg;
} i2s_ctrl1_t;

/**
 * \brief           I2S mclk set0 register at offet 0x08
 */
typedef union i2s_mclk_set0_s {
    struct i2s_mclk_set0_b {
        uint32_t mclk_isel : 3;                 /*!< i2s internal mclk selecttion */
        uint32_t reserved  : 29;                /*!< reserved bits */
    } bit;

    uint32_t reg;
} i2s_mclk_set0_t;

/**
 * \brief           I2S mclk set1 register at offet 0x0C
 */
typedef union i2s_mclk_set1_s {
    struct i2s_mclk_set1_b {
        uint32_t mclk_div : 4;                  /*!< i2s output mclk divider */
        uint32_t reserved : 28;                 /*!< reserved bits */
    } bit;

    uint32_t reg;
} i2s_mclk_se1_t;

/**
 * \brief           I2S master set0 register at offet 0x14
 */
typedef union i2s_ms_set0_s {
    struct i2s_ms_set0_b {
        uint32_t cfg_bck_osr : 2;               /*!< ratio of MCLK and BCLK */
        uint32_t cfg_i2s_mod : 2;               /*!< i2s tranceiver mode */
        uint32_t cfg_i2s_fmt : 2;               /*!< i2s format */
        uint32_t cfg_bck_len : 2;               /*!< bit length of bclk per channel */
        uint32_t cfg_txd_wid : 2;               /*!< sample width for i2s transmited sample */
        uint32_t cfg_rxd_wid : 2;               /*!< sample width for i2s received sample */
        uint32_t cfg_txd_chn : 2;               /*!< tx sample format in xdma */
        uint32_t cfg_rxd_chn : 2;               /*!< rx sample format in xdma */
        uint32_t cfg_i2s_tst : 8;               /*!< i2s test mode configuration */
        uint32_t reserved    : 4;               /*!< reserved bits */
        uint32_t cfg_dbg_sel : 4;               /*!< debug signal selection */
    } bit;

    uint32_t reg;
} i2s_ms_set0_t;

/**
 * \brief           I2S total register
 */
typedef struct {
    __IO i2s_ctrl0_t ms_ctl0;                   /*!< 0x00 i2s master control0 */
    __IO i2s_ctrl1_t ms_ctl1;                   /*!< 0x04 i2s master control1 */
    __IO i2s_mclk_set0_t mclk_set0;             /*!< 0x08 i2s mclk set0 */
    __IO i2s_mclk_se1_t mclk_set1;              /*!< 0x0C i2s mclk set1 */
    __IO uint32_t reserve0;                     /*!< 0x10 reserve*/
    __IO i2s_ms_set0_t ms_set0;                 /*!< 0x14 i2s master set0 */
    __IO uint32_t reserve1;                     /*!< 0x18 reserve */
    __IO uint32_t reserve2;                     /*!< 0x1C reserve */
    __IO uint32_t reserve3;                     /*!< 0x20 reserve */
    __IO uint32_t reserve4;                     /*!< 0x24 reserve */
    __IO uint32_t reserve5;                     /*!< 0x28 reserve */
    __IO uint32_t reserve6;                     /*!< 0x2C reserve */
    __IO uint32_t reserve7;                     /*!< 0x30 reserve */
    __IO uint32_t reserve8;                     /*!< 0x34 reserve */
    __IO uint32_t reserve9;                     /*!< 0x38 reserve */
    __IO uint32_t reserve10;                    /*!< 0x3C reserve */
    __IO uint32_t rdma_ctl0;                    /*!< 0x40 i2s rdma control0 */
    __IO uint32_t rdma_ctl1;                    /*!< 0x44 i2s rdma control1 */
    __IO uint32_t rdma_set0;                    /*!< 0x48 i2s rdma set0 */
    __IO uint32_t rdma_set1;                    /*!< 0x4C i2s rdma set1 */
    __IO uint32_t reserve11;                    /*!< 0x50 reserve */
    __IO uint32_t reserve12;                    /*!< 0x54 reserve */
    __I uint32_t rdma_r0;                       /*!< 0x58 i2s rdma r0 */
    __I uint32_t rdma_r1;                       /*!< 0x5C i2s rdma r1 */
    __IO uint32_t wdma_ctl0;                    /*!< 0x60 i2s wdma control0 */
    __IO uint32_t wdma_ctl1;                    /*!< 0x64 i2s wdma control1 */
    __IO uint32_t wdma_set0;                    /*!< 0x68 i2s wdma set0 */
    __IO uint32_t wdma_set1;                    /*!< 0x6C i2s wdma set1 */
    __IO uint32_t reserve13;                    /*!< 0x70 reserve */
    __IO uint32_t reserve14;                    /*!< 0x74 reserve */
    __I uint32_t wdma_r0;                       /*!< 0x78 i2s wdma r0 */
    __I uint32_t wdma_r1;                       /*!< 0x7C i2s wdma r1 */
    __IO uint32_t reserve15;                    /*!< 0x80 reserve */
    __IO uint32_t reserve16;                    /*!< 0x84 reserve */
    __IO uint32_t reserve17;                    /*!< 0x88 reserve */
    __IO uint32_t reserve18;                    /*!< 0x8C reserve */
    __IO uint32_t reserve19;                    /*!< 0x90 reserve */
    __IO uint32_t reserve20;                    /*!< 0x94 reserve */
    __IO uint32_t reserve21;                    /*!< 0x98 reserve */
    __IO uint32_t reserve22;                    /*!< 0x9C reserve */
    __IO uint32_t int_clear;                    /*!< 0xA0 i2s interrupt clear */
    __IO uint32_t int_mask;                     /*!< 0xA4 i2s interrupt mask */
    __I uint32_t int_status;                    /*!< 0xA8 i2s interrupt status */
} i2s_t;

/**
 * \brief           I2S rdma_ctl0 (0x40) bit definition
 */
#define I2S_RDMA_ENABLE_SHFT 0
#define I2S_RDMA_ENABLE_MASK (0x01UL << I2S_RDMA_ENABLE_SHFT)

/**
 * \brief           I2S rdma_ctl1 (0x44) bit definition
 */
#define I2S_RDMA_RESET_SHFT 0
#define I2S_RDMA_RESET_MASK (0x01UL << I2S_CFG_I2S_TST_SHFT)

/**
 * \brief           I2S rdma_set0 (0x48) bit definition
 */
#define I2S_RDMA_SEG_SIZE_SHFT 0
#define I2S_RDMA_SEG_SIZE_MASK (0x0000FFFFUL << I2S_RDMA_SEG_SIZE_SHFT)
#define I2S_RDMA_BLK_SIZE_SHFT 16
#define I2S_RDMA_BLK_SIZE_MASK (0x0000FFFFUL << I2S_RDMA_BLK_SIZE_SHFT)

/**
 * \brief           I2S rdma_set1 (0x4C) bit definition
 */
#define I2S_WDMA_ENABLE_SHFT 0
#define I2S_WDMA_ENABLE_MASK (0x01UL << I2S_WDMA_ENABLE_SHFT)

/**
 * \brief           I2S wdma_ctl1 (0x64) bit definition
 */
#define I2S_WDMA_RESET_SHFT 0
#define I2S_WDMA_RESET_MASK (0x01UL << I2S_WDMA_RESET_SHFT)

/**
 * \brief           I2S wdma_set0 (0x68) bit definition
 */
#define I2S_WDMA_SEG_SIZE_SHFT 0
#define I2S_WDMA_SEG_SIZE_MASK (0x0000FFFFUL << I2S_WDMA_SEG_SIZE_SHFT)
#define I2S_WDMA_BLK_SIZE_SHFT 16
#define I2S_WDMA_BLK_SIZE_MASK (0x0000FFFFUL << I2S_WDMA_BLK_SIZE_SHFT)

/**
 * \brief           I2S int_clear (0xA0) bit definition
 */
#define I2S_RDMA_IRQ_CLR_SHFT     0
#define I2S_RDMA_IRQ_CLR_MASK     (0x01UL << I2S_RDMA_IRQ_CLR_SHFT)
#define I2S_RDMA_ERR_IRQ_CLR_SHFT 1
#define I2S_RDMA_ERR_IRQ_CLR_MASK (0x01UL << I2S_RDMA_ERR_IRQ_CLR_SHFT)
#define I2S_WDMA_IRQ_CLR_SHFT     2
#define I2S_WDMA_IRQ_CLR_MASK     (0x01UL << I2S_WDMA_IRQ_CLR_SHFT)
#define I2S_WDMA_ERR_IRQ_CLR_SHFT 3
#define I2S_WDMA_ERR_IRQ_CLR_MASK (0x01UL << I2S_WDMA_ERR_IRQ_CLR_SHFT)

/**
 * \brief           I2S int_mask (0xA4) bit definition
 */
#define I2S_RDMA_IRQ_MASK_SHFT     0
#define I2S_RDMA_IRQ_MASK_MASK     (0x01UL << I2S_RDMA_IRQ_MASK_SHFT)
#define I2S_RDMA_ERR_IRQ_MASK_SHFT 1
#define I2S_RDMA_ERR_IRQ_MASK_MASK (0x01UL << I2S_RDMA_ERR_IRQ_MASK_SHFT)
#define I2S_WDMA_IRQ_MASK_SHFT     2
#define I2S_WDMA_IRQ_MASK_MASK     (0x01UL << I2S_WDMA_IRQ_MASK_SHFT)
#define I2S_WDMA_ERR_IRQ_MASK_SHFT 3
#define I2S_WDMA_ERR_IRQ_MASK_MASK (0x01UL << I2S_WDMA_ERR_IRQ_MASK_SHFT)

/**
 * \brief           I2S int_status (0xA8) bit definition
 */
#define I2S_RDMA_IRQ_STATUS_SHFT     0
#define I2S_RDMA_IRQ_STATUS_MASK     (0x01UL << I2S_RDMA_IRQ_STATUS_SHFT)
#define I2S_RDMA_ERR_IRQ_STATUS_SHFT 1
#define I2S_RDMA_ERR_IRQ_STATUS_MASK (0x01UL << I2S_RDMA_ERR_IRQ_STATUS_SHFT)
#define I2S_WDMA_IRQ_STATUS_SHFT     0
#define I2S_WDMA_IRQ_STATUS_MASK     (0x01UL << I2S_WDMA_IRQ_STATUS_SHFT)
#define I2S_WDMA_ERR_IRQ_STATUS_SHFT 1
#define I2S_WDMA_ERR_IRQ_STATUS_MASK (0x01UL << I2S_WDMA_ERR_IRQ_STATUS_SHFT)

#ifdef __cplusplus
}
#endif

#endif /* End of I2S_REG_H */

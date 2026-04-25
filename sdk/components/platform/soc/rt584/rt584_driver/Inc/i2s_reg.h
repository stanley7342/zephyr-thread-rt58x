/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           i2s_reg.h
 * \brief          I2S register definition header
 */
/*
 * Author:         Kc.tseng
 */



#ifndef I2S_REG_H
#define I2S_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           I2S master control 0 register at offet 0x00
 */
typedef union i2s_ms_ctl0_s {
    struct i2s_ms_ctl0_b {
        uint32_t cfg_i2s_ena       : 1;         /*!< i2s enable */
        uint32_t cfg_mck_ena       : 1;         /*!< mclk enable bits */
        uint32_t cfg_pdm_tx_en_mux : 1;         /*!< pdm tx en mux */
        uint32_t reserved0         : 5;         /*!< reserved bits */
        uint32_t cfg_i2s_ck_free   : 2;         /*!< configurate clock free run */
        uint32_t reserved1         : 22;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_ms_ctl0_t;

/**
 * \brief           I2S master control 1 register at offet 0x04
 */
typedef union i2s_ms_ctl1_s {
    struct i2s_ms_ctl1_b {
        uint32_t cfg_i2s_rst : 1;               /*!< i2s reset */
        uint32_t reserved    : 31;              /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_ms_ctl1_t;

/**
 * \brief           I2S mclk setting 0 register at offet 0x08
 */
typedef union i2s_mclk_set0_s {
    struct i2s_mclk_set0_b {
        uint32_t cfg_mck_isel : 3;              /*!< internal mclk rate selection */
        uint32_t reserved     : 29;             /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_mclk_set0_t;

/**
 * \brief           I2S mclk setting 1 register at offet 0x0C
 */
typedef union i2s_mclk_set1_s {
    struct i2s_mclk_set1_b {
        uint32_t cfg_mck_div : 4;               /*!< output mclk divider */
        uint32_t reserved    : 28;              /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_mclk_set1_t;

/**
 * \brief           I2S mclk setting 2 register at offet 0x10
 */
typedef union i2s_mclk_set2_s {
    struct i2s_mclk_set2_b {
        uint32_t cfg_mck_fra : 16;              /*!< mclk divider fractional value */
        uint32_t cfg_mck_int : 8;               /*!< mclk divider interger value */
        uint32_t reserved    : 8;               /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_mclk_set2_t;

/**
 * \brief           I2S master setting 0 register at offet 0x14
 */
typedef union i2s_ms_set0_s {
    struct i2s_ms_set0_b {
        uint32_t cfg_bck_osr : 2;               /*!< ratio of mclk and bclk */
        uint32_t cfg_i2s_mod : 2;               /*!< i2s tranceiver mode */
        uint32_t cfg_i2s_fmt : 2;               /*!< i2s format */
        uint32_t cfg_bck_len : 2;               /*!< bit length of bclk per channel */
        uint32_t cfg_txd_wid : 2;               /*!< sample width for i2s transmited sample */
        uint32_t cfg_rxd_wid : 2;               /*!< sample width for i2s received sample */
        uint32_t cfg_txd_chn : 2;               /*!< tx sample format in xdma */
        uint32_t cfg_rxd_chn : 2;               /*!< rx sample format in xdma */
        uint32_t cfg_i2s_tst : 8;               /*!< is2 test mode configuration */
        uint32_t reserved    : 4;               /*!< reserved bits */
        uint32_t cfg_dbg_sel : 4;               /*!< debug signal selection */
    } bit;
    uint32_t reg;
} i2s_ms_set0_t;

/**
 * \brief           I2S rdma control 0 register at offet 0x40
 */
typedef union i2s_rdma_ctl0_s {
    struct i2s_rdma_ctl0_b {
        uint32_t cfg_i2s_rdma_ctl0 : 4;         /*!< start xdma for memory access */
        uint32_t reserved          : 28;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_rdma_ctl0_t;

/**
 * \brief           I2S rdma control 1 register at offet 0x44
 */
typedef union i2s_rdma_ctl1_s {
    struct i2s_rdma_ctl1_b {
        uint32_t cfg_i2s_rdma_ctl1 : 1;         /*!< software reset xdma */
        uint32_t reserved          : 31;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_rdma_ctl1_t;

/**
 * \brief           I2S rdma setting 2 register at offet 0x50
 */
typedef union i2s_rdma_set2_s {
    struct i2s_rdma_set2_b {
        uint32_t cfg_i2s_rdma_set2 : 8;         /*!< rdma settings */
        uint32_t reserved          : 24;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_rdma_set2_t;

/**
 * \brief           I2S wdma control 0 register at offet 0x60
 */
typedef union i2s_wdma_ctl0_s {
    struct i2s_wdma_ctl0_b {
        uint32_t cfg_i2s_wdma_ctl0 : 4;         /*!< start xdma for memory access */
        uint32_t reserved          : 28;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_wdma_ctl0_t;

/**
 * \brief           I2S wdma control 1 register at offet 0x64
 */
typedef union i2s_wdma_ctl1_s {
    struct i2s_wdma_ctl1_b {
        uint32_t cfg_i2s_wdma_ctl1 : 1;         /*!< software reset xdma */
        uint32_t reserved          : 31;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_wdma_ctl1_t;

/**
 * \brief           I2S wdma setting 2 register at offet 0x70
 */
typedef union i2s_wdma_set2_s {
    struct i2s_wdma_set2_b {
        uint32_t cfg_i2s_wdma_set2 : 8;         /*!< rdma settings */
        uint32_t reserved          : 24;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} i2s_wdma_set2_t;

/**
 * \brief           I2S total register
 */
typedef struct {
    __IO  i2s_ms_ctl0_t   ms_ctl0;          /*!< 0x00 master control register 0 */
    __IO  i2s_ms_ctl1_t   ms_ctl1;          /*!< 0x04 master control register 1 */
    __IO  i2s_mclk_set0_t mclk_set0;        /*!< 0x08 mclk setting register 0 */
    __IO  i2s_mclk_set1_t mclk_set1;        /*!< 0x0C mclk setting register 1 */
    __IO  i2s_mclk_set2_t mclk_set2;        /*!< 0x10 mclk setting register 2 */
    __IO  i2s_ms_set0_t   ms_set0;          /*!< 0x14 master setting register 0 */
    __IO  uint32_t        resvd_1;          /*!< 0x17 reserved register */
    __IO  uint32_t        resvd_2;          /*!< 0x1C reserved register */
    __IO  uint32_t        resvd_3;          /*!< 0x20 reserved register */
    __IO  uint32_t        resvd_4;          /*!< 0x24 reserved register */
    __IO  uint32_t        resvd_5;          /*!< 0x28 reserved register */
    __IO  uint32_t        resvd_6;          /*!< 0x2C reserved register */
    __IO  uint32_t        resvd_7;          /*!< 0x30 reserved register */
    __IO  uint32_t        resvd_8;          /*!< 0x34 reserved register */
    __IO  uint32_t        resvd_9;          /*!< 0x38 reserved register */
    __IO  uint32_t        resvd_10;         /*!< 0x3C reserved register */
    __IO  i2s_rdma_ctl0_t rdma_ctl0;        /*!< 0x40 rdma control register 0 */
    __IO  i2s_rdma_ctl1_t rdma_ctl1;        /*!< 0x44 rdma control register 1 */
    __IO  uint32_t        rdma_set0;        /*!< 0x48 rdma setting register 0 */
    __IO  uint32_t        rdma_set1;        /*!< 0x4C rdma setting register 1 */
    __IO  i2s_rdma_set2_t rdma_set2;        /*!< 0x50 rdma setting register 2 */
    __IO  uint32_t        resvd_11;         /*!< 0x54 reserved register */
    __I   uint32_t        rdma_r0;          /*!< 0x58 rdma r0 */
    __I   uint32_t        rdma_r1;          /*!< 0x5C rdma r1 */
    __IO  i2s_wdma_ctl0_t wdma_ctl0;        /*!< 0x60 wdma control register 0 */
    __IO  i2s_wdma_ctl1_t wdma_ctl1;        /*!< 0x64 wdma control register 1 */
    __IO  uint32_t        wdma_set0;        /*!< 0x68 wdma setting register 0 */
    __IO  uint32_t        wdma_set1;        /*!< 0x6C wdma setting register 1 */
    __IO  i2s_wdma_set2_t wdma_set2;        /*!< 0x70 wdma setting register 2 */
    __IO  uint32_t        resvd_12;         /*!< 0x74 reserved register */
    __I   uint32_t        wdma_r0;          /*!< 0x78 wdma r0 */
    __I   uint32_t        wdma_r1;          /*!< 0x7C wdma r1 */
    __IO  uint32_t        resvd_13;         /*!< 0x80 reserved register */
    __IO  uint32_t        resvd_14;         /*!< 0x84 reserved register */
    __IO  uint32_t        resvd_15;         /*!< 0x88 reserved register */
    __IO  uint32_t        resvd_16;         /*!< 0x8C reserved register */
    __IO  uint32_t        resvd_17;         /*!< 0x90 reserved register */
    __IO  uint32_t        resvd_18;         /*!< 0x94 reserved register */
    __IO  uint32_t        resvd_19;         /*!< 0x98 reserved register */
    __IO  uint32_t        resvd_20;         /*!< 0x9C reserved register */
    __IO  uint32_t        int_clear;        /*!< 0xA0 interrupt clear register */
    __IO  uint32_t        int_mask;         /*!< 0xA4 interrupt mask register*/
    __I   uint32_t        int_status;       /*!< 0xA8 interrupt status register */
} i2s_t;

/**
 * \brief           I2S rdma_ctl0 (0x40) bit definition
 */
#define I2S_RDMA_ENABLE_SHFT          0
#define I2S_RDMA_ENABLE_MASK          (0x01UL << I2S_RDMA_ENABLE_SHFT)

/**
 * \brief           I2S rdma_ctl1 (0x44) bit definition
 */
#define I2S_RDMA_RESET_SHFT           0
#define I2S_RDMA_RESET_MASK           (0x01UL << I2S_CFG_I2S_TST_SHFT)

/**
 * \brief           I2S rdma_set0 (0x48) bit definition
 */
#define I2S_RDMA_SEG_SIZE_SHFT        0
#define I2S_RDMA_SEG_SIZE_MASK        (0x0000FFFFUL << I2S_RDMA_SEG_SIZE_SHFT)
#define I2S_RDMA_BLK_SIZE_SHFT        16
#define I2S_RDMA_BLK_SIZE_MASK        (0x0000FFFFUL << I2S_RDMA_BLK_SIZE_SHFT)

/**
 * \brief           I2S rdma_set1 (0x4C) bit definition
 */
#define I2S_WDMA_ENABLE_SHFT          0
#define I2S_WDMA_ENABLE_MASK          (0x01UL << I2S_WDMA_ENABLE_SHFT)

/**
 * \brief           I2S wdma_ctl1 (0x64) bit definition
 */
#define I2S_WDMA_RESET_SHFT           0
#define I2S_WDMA_RESET_MASK           (0x01UL << I2S_WDMA_RESET_SHFT)

/**
 * \brief           I2S wdma_set0 (0x68) bit definition
 */
#define I2S_WDMA_SEG_SIZE_SHFT        0
#define I2S_WDMA_SEG_SIZE_MASK        (0x0000FFFFUL << I2S_WDMA_SEG_SIZE_SHFT)
#define I2S_WDMA_BLK_SIZE_SHFT        16
#define I2S_WDMA_BLK_SIZE_MASK        (0x0000FFFFUL << I2S_WDMA_BLK_SIZE_SHFT)

/**
 * \brief           I2S int_clear (0xA0) bit definition
 */
#define I2S_RDMA_IRQ_CLR_SHFT         0
#define I2S_RDMA_IRQ_CLR_MASK         (0x01UL << I2S_RDMA_IRQ_CLR_SHFT)
#define I2S_RDMA_ERR_IRQ_CLR_SHFT     1
#define I2S_RDMA_ERR_IRQ_CLR_MASK     (0x01UL << I2S_RDMA_ERR_IRQ_CLR_SHFT)
#define I2S_WDMA_IRQ_CLR_SHFT         2
#define I2S_WDMA_IRQ_CLR_MASK         (0x01UL << I2S_WDMA_IRQ_CLR_SHFT)
#define I2S_WDMA_ERR_IRQ_CLR_SHFT     3
#define I2S_WDMA_ERR_IRQ_CLR_MASK     (0x01UL << I2S_WDMA_ERR_IRQ_CLR_SHFT)

/**
 * \brief           I2S int_mask (0xA4) bit definition
 */
#define I2S_RDMA_IRQ_MASK_SHFT        0
#define I2S_RDMA_IRQ_MASK_MASK        (0x01UL << I2S_RDMA_IRQ_MASK_SHFT)
#define I2S_RDMA_ERR_IRQ_MASK_SHFT    1
#define I2S_RDMA_ERR_IRQ_MASK_MASK    (0x01UL << I2S_RDMA_ERR_IRQ_MASK_SHFT)
#define I2S_WDMA_IRQ_MASK_SHFT        2
#define I2S_WDMA_IRQ_MASK_MASK        (0x01UL << I2S_WDMA_IRQ_MASK_SHFT)
#define I2S_WDMA_ERR_IRQ_MASK_SHFT    3
#define I2S_WDMA_ERR_IRQ_MASK_MASK    (0x01UL << I2S_WDMA_ERR_IRQ_MASK_SHFT)

/**
 * \brief           I2S int_status (0xA8) bit definition
 */
#define I2S_RDMA_IRQ_STATUS_SHFT      0
#define I2S_RDMA_IRQ_STATUS_MASK      (0x01UL << I2S_RDMA_IRQ_STATUS_SHFT)
#define I2S_RDMA_ERR_IRQ_STATUS_SHFT  1
#define I2S_RDMA_ERR_IRQ_STATUS_MASK  (0x01UL << I2S_RDMA_ERR_IRQ_STATUS_SHFT)
#define I2S_WDMA_IRQ_STATUS_SHFT      0
#define I2S_WDMA_IRQ_STATUS_MASK      (0x01UL << I2S_WDMA_IRQ_STATUS_SHFT)
#define I2S_WDMA_ERR_IRQ_STATUS_SHFT  1
#define I2S_WDMA_ERR_IRQ_STATUS_MASK  (0x01UL << I2S_WDMA_ERR_IRQ_STATUS_SHFT)


#ifdef __cplusplus
}
#endif

#endif /* End of I2S_REG_H */

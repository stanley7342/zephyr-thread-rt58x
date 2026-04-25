/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           sadc_reg.h
 * \brief          SADC register definition header file
 */
/*
 * This file is part of library_name.
 * Author:          Kc.tseng
 */

#ifndef SADC_REG_H
#define SADC_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Sadc control register 0 at offet 0x00
 */
typedef union sadc_ctrl0_s {
    struct sadc_ctrl0_b {
        uint32_t cfg_sadc_ena     : 1;          /*!< Enable SADC */
        uint32_t cfg_sadc_vga_ena : 1;          /*!< Enable SADC VGA */
        uint32_t cfg_sadc_ldo_ena : 1;          /*!< Enable SADC LDO */
        uint32_t reserved1        : 5;          /*!< reserved bits */
        uint32_t cfg_sadc_ck_free : 2;          /*!< SADC clock free run setting */
        uint32_t reserved2        : 22;         /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_ctrl0_t;

/**
 * \brief           Sadc control register 1 at offet 0x04
 */
typedef union sadc_ctrl1_s {
    struct sadc_ctrl1_b {
        uint32_t cfg_sadc_rst       : 1;        /*!< SADC reset */
        uint32_t reserved1          : 7;        /*!< reserved bits */
        uint32_t cfg_sadc_afifo_rst : 1;        /*!< Reset SADC AFIFO*/
        uint32_t reserved2          : 23;       /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_ctrl1_t;

/**
 * \brief           Sadc control register 2 at offet 0x08
 */
typedef union sadc_ctrl2_s {
    struct sadc_ctrl2_b {
        uint32_t cfg_sadc_start : 1;            /*!< SADC start */
        uint32_t reserved       : 31;           /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_ctrl2_t;

/**
 * \brief           Sadc parameter setting register 0 at offet 0x0C
 */
typedef union sadc_set0_s {
    struct sadc_set0_b {
        uint32_t cfg_sadc_smp_mode     : 1;     /*!< SADC sample rate mode */
        uint32_t cfg_sadc_tmr_cksel    : 1;     /*!< SADC timer mode clock select */
        uint32_t cfg_sadc_afifo_ckpsel : 1;     /*!< AFIFO Clock phase selection */
        uint32_t cfg_sadc_dbg_sel      : 4;     /*!< Debug monitor selection */
        uint32_t reserved              : 9;     /*!< reserved bits */
        uint32_t cfg_sadc_tmr_ckdiv    : 16;    /*!< Timer rate configuration */
    } bit;
    uint32_t reg;
} sadc_set0_t;

/**
 * \brief           Sadc parameter setting register 1 at offet 0x10
 */
typedef union sadc_set1_s {
    struct sadc_set1_b {
        uint32_t cfg_sadc_bit     : 4;          /*!< SADC output resolution */
        uint32_t cfg_sadc_chx_sel : 4;          /*!< At test mode, select from
                                                 CH0 to CH9 */
        uint32_t cfg_sadc_osr     : 4;          /*!< Oversample rate selection */
        uint32_t cfg_sadc_tst     : 4;          /*!< SADC test mode setting */
        uint32_t cfg_sadc_val_tst : 12;         /*!< Set SADC adjust value */
        uint32_t reserved         : 4;          /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_set1_t;

/**
 * \brief           Sadc pn select channel setting register,
 *                  Channel 0 at offet 0x20
 *                  Channel 1 at offet 0x30
 *                  Channel 2 at offet 0x40
 *                  Channel 3 at offet 0x50
 *                  Channel 4 at offet 0x60
 *                  Channel 5 at offet 0x70
 *                  Channel 6 at offet 0x80
 *                  Channel 7 at offet 0x90
 *                  Channel 8 at offet 0xA0
 *                  Channel 9 at offet 0xB0
 */
typedef union sadc_pnsel_ch_s {
    struct sadc_pnsel_ch_b {
        uint32_t cfg_sadc_psel_ch   : 4;        /*!< SADC P channel selection */
        uint32_t cfg_sadc_nsel_ch   : 4;        /*!< SADC N channel selection */
        uint32_t cfg_sadc_gain_ch   : 6;        /*!< VGA gain selection */
        uint32_t cfg_sadc_ref_in_ch : 2;        /*!< SADC setting */
        uint32_t cfg_sadc_pull_ch   : 8;        /*!< P/N channel pull setting */
        uint32_t cfg_sadc_tacq_ch   : 3;        /*!< SADC result acquisition time
                                                 for system clock 32M */
        uint32_t reserved1          : 1;        /*!< reserved bits */
        uint32_t cfg_sadc_edly_ch   : 3;        /*!< SADC end delay time for
                                                 system clock 32M */
        uint32_t reserved2          : 1;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_pnsel_ch_t;

/**
 * \brief           Sadc channel setting register,
 *                  Channel 0 at offet 0x24
 *                  Channel 1 at offet 0x34
 *                  Channel 2 at offet 0x44
 *                  Channel 3 at offet 0x54
 *                  Channel 4 at offet 0x64
 *                  Channel 5 at offet 0x74
 *                  Channel 6 at offet 0x84
 *                  Channel 7 at offet 0x94
 *                  Channel 8 at offet 0xA4
 *                  Channel 9 at offet 0xB4
 */
typedef union sadc_set_ch_s {
    struct sadc_set_ch_b {
        uint32_t reserved          : 31;        /*!< reserved bits */
        uint32_t cfg_sadc_burst_ch : 1;         /*!< SADC burst mode selection */
    } bit;
    uint32_t reg;
} sadc_set_ch_t;

/**
 * \brief           Sadc channel threshold setting register,
 *                  Channel 0 at offet 0x28
 *                  Channel 1 at offet 0x38
 *                  Channel 2 at offet 0x48
 *                  Channel 3 at offet 0x58
 *                  Channel 4 at offet 0x68
 *                  Channel 5 at offet 0x78
 *                  Channel 6 at offet 0x88
 *                  Channel 7 at offet 0x98
 *                  Channel 8 at offet 0xA8
 *                  Channel 9 at offet 0xB8
 */
typedef union sadc_thd_ch_s {
    struct sadc_thd_ch_b {
        uint32_t cfg_sadc_lthd_ch : 14;         /*!< SADC low threshold */
        uint32_t reserved1        : 2;          /*!< reserved bits */
        uint32_t cfg_sadc_hthd_ch : 14;         /*!< SADC high threshold */
        uint32_t reserved2        : 2;          /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_thd_ch_t;

/**
 * \brief           Sadc analog setting register 0 at offet 0xBC
 */
typedef union sadc_ana_set0_s {
    struct sadc_ana_set0_b {
        uint32_t aux_adc_debug     : 1;         /*!< SADC analog settings */
        uint32_t aux_adc_mode      : 1;         /*!< SADC analog settings */
        uint32_t aux_adc_outputstb : 1;         /*!< SADC analog settings */
        uint32_t aux_adc_ospn      : 1;         /*!< SADC analog settings */
        uint32_t aux_adc_clk_sel   : 2;         /*!< SADC analog settings */
        uint32_t aux_adc_mcap      : 2;         /*!< SADC analog settings */
        uint32_t aux_adc_mdly      : 2;         /*!< SADC analog settings */
        uint32_t aux_adc_sel_duty  : 2;         /*!< SADC analog settings */
        uint32_t aux_adc_os        : 2;         /*!< SADC analog settings */
        uint32_t reserved1         : 2;         /*!< reserved bits */
        uint32_t aux_adc_br        : 4;         /*!< SADC analog settings */
        uint32_t aux_adc_pw        : 3;         /*!< SADC analog settings */
        uint32_t reserved2         : 1;         /*!< reserved bits */
        uint32_t aux_adc_stb_bit   : 3;         /*!< SADC analog settings */
        uint32_t reserved3         : 1;         /*!< reserved bits */
        uint32_t aux_pw            : 3;         /*!< SADC analog settings */
        uint32_t reserved4         : 1;         /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_ana_set0_t;

/**
 * \brief           Sadc analog setting register 1 at offet 0xC0
 */
typedef union sadc_ana_set1_s {
    struct sadc_ana_set1_b {
        uint32_t aux_vga_cmsel       : 4;       /*!< SADC analog settings */
        uint32_t aux_vga_comp        : 2;       /*!< SADC analog settings */
        uint32_t aux_vga_sin         : 2;       /*!< SADC analog settings */
        uint32_t aux_vga_lout        : 1;       /*!< SADC analog settings */
        uint32_t aux_vga_sw_vdd      : 1;       /*!< SADC analog settings */
        uint32_t aux_vga_vldo        : 2;       /*!< SADC analog settings */
        uint32_t aux_vga_acm         : 4;       /*!< SADC analog settings */
        uint32_t aux_vga_pw          : 6;       /*!< SADC analog settings */
        uint32_t aux_dc_adj          : 2;       /*!< SADC analog settings */
        uint32_t aux_test_mode       : 1;       /*!< SADC analog settings */
        uint32_t cfg_en_clkaux       : 1;       /*!< SADC analog settings */
        uint32_t aux_vga_test_aio_en : 1;       /*!< SADC analog settings */
        uint32_t reserved1           : 5;       /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_ana_set1_t;

/**
 * \brief           Sadc wdma control register 0 at offet 0x100
 */
typedef union sadc_wdma_ctl0_s {
    struct sadc_wdma_ctl0_b {
        uint32_t cfg_sadc_wdma_ctl0 : 1;        /*!< SADC wdma start */
        uint32_t reserved           : 31;       /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_wdma_ctl0_t;

/**
 * \brief           Sadc wdma control register 1 at offet 0x104
 */
typedef union sadc_wdma_ctl1_s {
    struct sadc_wdma_ctl1_b {
        uint32_t cfg_sadc_wdma_ctl1 : 1;        /*!< SADC wdma reset */
        uint32_t reserved           : 31;       /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_wdma_ctl1_t;

/**
 * \brief           Sadc wdma setting register 0 at offet 0x108
 */
typedef union sadc_wdma_set0_s {
    struct sadc_wdma_set0_b {
        uint32_t cfg_sadc_seg_size : 16;        /*!< Wdma segment size */
        uint32_t cfg_sadc_blk_size : 16;        /*!< Wdma blokc size */
    } bit;
    uint32_t reg;
} sadc_wdma_set0_t;

/**
 * \brief           Sadc wdma setting register 2 at offet 0x110
 */
typedef union sadc_wdma_set2_s {
    struct sadc_wdma_set2_b {
        uint32_t cfg_sadc_init_addr    : 1;     /*!< Wdma start address */
        uint32_t reserved1             : 3;     /*!< reserved bits */
        uint32_t cfg_sadc_dma_data_fmt : 2;     /*!< Wdma data formate */
        uint32_t reserved2             : 26;    /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_wdma_set2_t;

/**
 * \brief           Sadc interrupt setting register, at offet 0x110
 *                  sadc_int_clear at offet 0x120
 *                  sadc_int_mask at offet 0x124
 *                  sadc_int_status at offet 0x128
 */
typedef union sadc_int_s {
    struct sadc_int_b {
        uint32_t wdma         : 1;              /*!< Wdma interrupt */
        uint32_t wdma_error   : 1;              /*!< Wdma error interrupt */
        uint32_t done         : 1;              /*!< Done interrupt */
        uint32_t valid        : 1;              /*!< Valid interrupt */
        uint32_t mode_done    : 1;              /*!< Mode done interrupt */
        uint32_t reserved1    : 3;              /*!< reserved bits */
        uint32_t monitor_low  : 10;             /*!< Low threshold interrupt */
        uint32_t monitor_high : 10;             /*!< High threshold interrupt */
        uint32_t reserved2    : 4;              /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_int_t;

/**
 * \brief           Sadc R0 register at offet 0x12C
 */
typedef union sadc_r0_s {
    struct sadc_r0_b {
        uint32_t sadc_o     : 14;               /*!< SADC output value after digital
                                                 control and oversampling */
        uint32_t reserved1  : 2;                /*!< reserved bits */
        uint32_t sadc_o_chx : 4;                /*!< SADC channel index */
        uint32_t reserved2  : 12;               /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_r0_t;

/**
 * \brief           Sadc R1 register at offet 0x130
 */
typedef union sadc_r1_s {
    struct sadc_r1_b {
        uint32_t sadc_i_12b   : 12;             /*!< SADC output value after digital
                                                 control and No oversampling */
        uint32_t reserved     : 4;              /*!< reserved bits */
        uint32_t sadc_num_res : 16;             /*!< Number of SADC result write into
                                                 WDMA since last sadc_start trigger */
    } bit;
    uint32_t reg;
} sadc_r1_t;

/**
 * \brief           Sadc R2 register at offet 0x134
 */
typedef union sadc_r2_s {
    struct sadc_r2_b {
        uint32_t sadc_i_syn   : 12;             /*!< SADC result via AFIFO */
        uint32_t reserved1    : 4;              /*!< reserved bits */
        uint32_t sadc_busy    : 1;              /*!< SADC mode control busy */
        uint32_t sadc_ana_ena : 1;              /*!< SADC analog enable */
        uint32_t reserved2    : 14;             /*!< reserved bits */
    } bit;
    uint32_t reg;
} sadc_r2_t;

/**
 * \brief           Sadc total register
 */
typedef struct {
    __IO sadc_ctrl0_t     sadc_ctl0;            /*!< 0x00 control 0 register */
    __IO sadc_ctrl1_t     sadc_ctl1;            /*!< 0x04 control 1 register */
    __IO sadc_ctrl2_t     sadc_ctl2;            /*!< 0x08 control 2 register */
    __IO sadc_set0_t      sadc_set0;            /*!< 0x0C parameter setting register 0 */
    __IO sadc_set1_t      sadc_set1;            /*!< 0x10 parameter setting register 1 */
    __IO uint32_t         sadc_reserved1;       /*!< 0x14 reserved register */
    __IO uint32_t         sadc_reserved2;       /*!< 0x18 reserved register */
    __IO uint32_t         sadc_reserved3;       /*!< 0x1C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch0;       /*!< 0x20 channel 0 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch0;         /*!< 0x24 channel 0 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch0;         /*!< 0x28 channel 0 threshold */
    __IO uint32_t         sadc_reserved4;       /*!< 0x2C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch1;       /*!< 0x30 channel 1 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch1;         /*!< 0x34 channel 1 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch1;         /*!< 0x38 channel 1 threshold */
    __IO uint32_t         sadc_reserved5;       /*!< 0x3C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch2;       /*!< 0x40 channel 2 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch2;         /*!< 0x44 channel 2 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch2;         /*!< 0x48 channel 2 threshold */
    __IO uint32_t         sadc_reserved6;       /*!< 0x4C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch3;       /*!< 0x50 channel 3 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch3;         /*!< 0x54 channel 3 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch3;         /*!< 0x58 channel 3 threshold */
    __IO uint32_t         sadc_reserved7;       /*!< 0x5C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch4;       /*!< 0x60 channel 4 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch4;         /*!< 0x64 channel 4 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch4;         /*!< 0x68 channel 4 threshold */
    __IO uint32_t         sadc_reserved8;       /*!< 0x6C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch5;       /*!< 0x70 channel 5 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch5;         /*!< 0x74 channel 5 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch5;         /*!< 0x78 channel 5 threshold */
    __IO uint32_t         sadc_reserved9;       /*!< 0x7C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch6;       /*!< 0x80 channel 6 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch6;         /*!< 0x84 channel 6 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch6;         /*!< 0x88 channel 6 threshold */
    __IO uint32_t         sadc_reserved10;      /*!< 0x8C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch7;       /*!< 0x90 channel 7 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch7;         /*!< 0x94 channel 7 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch7;         /*!< 0x98 channel 7 threshold */
    __IO uint32_t         sadc_reserved11;      /*!< 0x9C reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch8;       /*!< 0xA0 channel 8 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch8;         /*!< 0xA4 channel 8 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch8;         /*!< 0xA8 channel 8 threshold */
    __IO uint32_t         sadc_reserved12;      /*!< 0xAC reserved register */
    __IO sadc_pnsel_ch_t  sadc_pnsel_ch9;       /*!< 0xB0 channel 9 PN settings */
    __IO sadc_set_ch_t    sadc_set_ch9;         /*!< 0xB4 channel 9 settings */
    __IO sadc_thd_ch_t    sadc_thd_ch9;         /*!< 0xB8 channel 9 threshold */
    __IO sadc_ana_set0_t  sadc_ana_set0;        /*!< 0xBC analog settings register */
    __IO sadc_ana_set1_t  sadc_ana_set1;        /*!< 0xC0 analog settings register */
    __IO uint32_t         sadc_reserved13;      /*!< 0xC4 reserved register */
    __IO uint32_t         sadc_reserved14;      /*!< 0xC8 reserved register */
    __IO uint32_t         sadc_reserved15;      /*!< 0xCC reserved register */
    __IO uint32_t         sadc_reserved16;      /*!< 0xD0 reserved register */
    __IO uint32_t         sadc_reserved17;      /*!< 0xD4 reserved register */
    __IO uint32_t         sadc_reserved18;      /*!< 0xD8 reserved register */
    __IO uint32_t         sadc_reserved19;      /*!< 0xDC reserved register */
    __IO uint32_t         sadc_reserved20;      /*!< 0xE0 reserved register */
    __IO uint32_t         sadc_reserved21;      /*!< 0xE4 reserved register */
    __IO uint32_t         sadc_reserved22;      /*!< 0xE8 reserved register */
    __IO uint32_t         sadc_reserved23;      /*!< 0xEC reserved register */
    __IO uint32_t         sadc_reserved24;      /*!< 0xF0 reserved register */
    __IO uint32_t         sadc_reserved25;      /*!< 0xF4 reserved register */
    __IO uint32_t         sadc_reserved26;      /*!< 0xF8 reserved register */
    __IO uint32_t         sadc_reserved27;      /*!< 0xFC reserved register */
    __IO sadc_wdma_ctl0_t sadc_wdma_ctl0;       /*!< 0x100 wdma control 0 register */
    __IO sadc_wdma_ctl1_t sadc_wdma_ctl1;       /*!< 0x104 wdma control 1 register */
    __IO sadc_wdma_set0_t sadc_wdma_set0;       /*!< 0x108 wdma settings 0 register*/
    __IO uint32_t         sadc_wdma_set1;       /*!< 0x10C wdma settings 1 register*/
    __IO sadc_wdma_set2_t sadc_wdma_set2;       /*!< 0x110 wdma settings 2 register*/
    __IO uint32_t         sadc_wdma_r0;         /*!< 0x114 wdma r0 */
    __IO uint32_t         sadc_wdma_r1;         /*!< 0x118 wdma r1 */
    __IO uint32_t         sadc_reserved28;      /*!< 0x11C reserved register */
    __IO sadc_int_t       sadc_int_clear;       /*!< 0x120 sadc interrupt clear register */
    __IO sadc_int_t       sadc_int_mask;        /*!< 0x124 sadc interrupt mask register */
    __IO sadc_int_t       sadc_int_status;      /*!< 0x128 sadc interrupt status register */
    __IO sadc_r0_t        sadc_r0;              /*!< 0x12C sadc r0 */
    __IO sadc_r1_t        sadc_r1;              /*!< 0x130 sadc r1 */
    __IO sadc_r2_t        sadc_r2;              /*!< 0x134 sadc r2 */
} sadc_t;

#ifdef __cplusplus
}
#endif

#endif /* End of SADC_REG_H */

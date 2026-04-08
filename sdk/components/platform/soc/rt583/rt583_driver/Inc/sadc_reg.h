/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            sadc_reg.h
 * \brief           sadc register header file
 */
/*
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
        uint32_t cfg_sadc_ena     : 1;          /*!< enable sadc bit */
        uint32_t cfg_sadc_vga_ena : 1;          /*!< enable SADC VGA, only work
                                                 when cfg_sadc_tst[2] = 1 */
        uint32_t cfg_sadc_ldo_ena : 1;          /*!< enable SADC LDO, only work
                                                 when cfg_sadc_tst[2] = 1 */
        uint32_t reserved1        : 5;          /*!< reserved bits */
        uint32_t cfg_sadc_ck_free : 2;          /*!< enable SADC clock free run */
        uint32_t reserved2        : 22;         /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_ctrl0_t;

/**
 * \brief           Sadc control register 1 at offet 0x04
 */
typedef union sadc_ctrl1_s {
    struct sadc_ctrl1_b {
        uint32_t cfg_sadc_rst       : 1;        /*!< reset SADC */
        uint32_t reserved1          : 7;        /*!< reserved bits */
        uint32_t cfg_sadc_afifo_rst : 1;        /*!< reset SADC AFIFO */
        uint32_t reserved2          : 23;       /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_ctrl1_t;

/**
 * \brief           Sadc control register 2 at offet 0x08
 */
typedef union sadc_ctrl2_s {
    struct sadc_ctrl2_b {
        uint32_t cfg_sadc_start : 1;            /*!< software start SADC */
        uint32_t reserved       : 31;           /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_ctrl2_t;

/**
 * \brief           Sadc setting register 0 at offet 0x0C
 */
typedef union sadc_set0_s {
    struct sadc_set0_b {
        uint32_t cfg_sadc_smp_mode     : 1;     /*!< SADC sample rate mode */
        uint32_t cfg_sadc_tmr_cksel    : 1;     /*!< SADC timer clock source
                                                 selection*/
        uint32_t cfg_sadc_afifo_ckpsel : 1;     /*!< SADC AFIFO clock phase
                                                 selection*/
        uint32_t cfg_sadc_dbg_sel      : 4;     /*!< SADC debug monitor
                                                 selection*/
        uint32_t reserved              : 9;     /*!< reserved bits */
        uint32_t cfg_sadc_tmr_ckdiv    : 16;    /*!< SADC timer rate
                                                 configuration*/
    } bit;

    uint32_t reg;
} sadc_set0_t;

/**
 * \brief           Sadc setting register 1 at offet 0x10
 */
typedef union sadc_set1_s {
    struct sadc_set1_b {
        uint32_t cfg_sadc_bit     : 4;          /*!< SADC output resolution */
        uint32_t cfg_sadc_chx_sel : 4;          /*!< At test mode, select from
                                                 CH0 to CH9 */
        uint32_t cfg_sadc_osr     : 4;          /*!< SADC Oversample rate
                                                 selection */
        uint32_t cfg_sadc_tst     : 4;          /*!< SADC test mode setting */
        uint32_t cfg_sadc_val_tst : 12;         /*!< Set SADC adjust value,
                                                 used for calibration */
        uint32_t reserved         : 4;          /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_set1_t;

/**
 * \brief           Sadc channelx pnsel setting register at offet 0x20, 0x30, 0x40,
 *                  0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0
 */
typedef union sadc_pnsel_ch_s {
    struct sadc_pnsel_ch_b {
        uint32_t cfg_sadc_psel_ch   : 4;        /*!< SADC P channel selection */
        uint32_t cfg_sadc_nsel_ch   : 4;        /*!< SADC N channel selection */
        uint32_t cfg_sadc_gain_ch   : 6;        /*!< SADC gain */
        uint32_t cfg_sadc_ref_in_ch : 2;        /*!< SADC ref */
        uint32_t cfg_sadc_pull_ch   : 8;        /*!< SADC Oversample rate */
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
 * \brief           Sadc channelx setting register at offet 0x24, 0x34, 0x44,
 *                  0x54, 0x64, 0x74, 0x84, 0x94, 0xA4, 0xB4
 */
typedef union sadc_set_ch_s {
    struct sadc_set_ch_b {
        uint32_t reserved          : 31;        /*!< reserved bits */
        uint32_t cfg_sadc_burst_ch : 1;         /*!< SADC burst mode selection */
    } bit;

    uint32_t reg;
} sadc_set_ch_t;

/**
 * \brief           Sadc channelx threshold register at offet 0x28, 0x38, 0x48,
 *                  0x58, 0x68, 0x78, 0x88, 0x98, 0xA8, 0xB8
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
        uint32_t cfg_aux_ana_set0 : 19;         /*!< SADC analog seting */
        uint32_t reserved         : 13;         /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_ana_set0_t;

/**
 * \brief           Sadc analog setting register 1 at offet 0xC0
 */
typedef union sadc_ana_set1_s {
    struct sadc_ana_set1_b {
        uint32_t cfg_aux_cmsel         : 4;     /*!< SADC analog seting */
        uint32_t cfg_aux_comp          : 2;     /*!< SADC analog seting */
        uint32_t cfg_aux_adc_outputstb : 1;     /*!< SADC analog seting */
        uint32_t cfg_aux_test_mode     : 1;     /*!< SADC analog seting */
        uint32_t cfg_aux_vldo          : 2;     /*!< SADC analog seting */
        uint32_t cfg_aux_clk_sel       : 2;     /*!< SADC analog seting */
        uint32_t reserved1             : 4;     /*!< reserved bits */
        uint32_t cfg_aux_pw            : 6;     /*!< SADC analog seting */
        uint32_t reserved2             : 3;     /*!< reserved bits */
        uint32_t cfg_en_clkaux         : 1;     /*!< SADC analog seting */
        uint32_t reserved3             : 6;     /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_ana_set1_t;

/**
 * \brief           Sadc wdma control register 0 at offet 0x100
 */
typedef union sadc_wdma_ctl0_s {
    struct sadc_wdma_ctl0_b {
        uint32_t cfg_sadc_wdma_ctl0 : 1;        /*!< SADC start xdma for memory
                                                 access*/
        uint32_t reserved           : 31;       /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_wdma_ctl0_t;

/**
 * \brief           Sadc wdma control register 1 at offet 0x104
 */
typedef union sadc_wdma_ctl1_s {
    struct sadc_wdma_ctl1_b {
        uint32_t cfg_sadc_wdma_ctl1 : 1;        /*!< SADC Software reset xdma */
        uint32_t reserved           : 31;       /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_wdma_ctl1_t;

/**
 * \brief           Sadc wdma setting register 0 at offet 0x108
 */
typedef union sadc_wdma_set0_s {
    struct sadc_wdma_set0_b {
        uint32_t cfg_sadc_seg_size : 16;        /*!< SADC wdma segment size */
        uint32_t cfg_sadc_blk_size : 16;        /*!< SADC block segment size */
    } bit;

    uint32_t reg;
} sadc_wdma_set0_t;

/**
 * \brief           Sadc wdma control register 2 at offet 0x110
 */
typedef union sadc_wdma_set2_s {
    struct sadc_wdma_set2_b {
        uint32_t cfg_sadc_init_addr    : 1;     /*!< SADC set initial address */
        uint32_t reserved1             : 3;     /*!< reserved bits */
        uint32_t cfg_sadc_dma_data_fmt : 2;     /*!< SADC DMA data format */
        uint32_t reserved2             : 26;    /*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_wdma_set2_t;

/**
 * \brief           Sadc interupt register at offet 0x120, 0x124, 0x128
 */
typedef union sadc_int_s {
    struct sadc_int_b {
        uint32_t wdma         : 1; /*!< SADC wdma interrupt */
        uint32_t wdma_error   : 1; /*!< SADC wdma error interrupt */
        uint32_t done         : 1; /*!< SADC done interrupt */
        uint32_t valid        : 1; /*!< SADC valid interrupt */
        uint32_t mode_done    : 1; /*!< SADC mode done interrupt */
        uint32_t reserved1    : 3;/*!< reserved bits */
        uint32_t monitor_low  : 10; /*!< SADC monitor low interrupt */
        uint32_t monitor_high : 10; /*!< SADC monitor high interrupt */
        uint32_t reserved2    : 4;/*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_int_t;

/**
 * \brief           Sadc r0 register at offet 0x12C
 */
typedef union sadc_r0_s {
    struct sadc_r0_b {
        uint32_t sadc_o     : 14; /*!< SADC output value after
                                                    digital control and
                                                    oversampling */
        uint32_t reserved1  : 2;/*!< reserved bits */
        uint32_t sadc_o_chx : 4; /*!< SADC channel index */
        uint32_t reserved2  : 12;/*!< reserved bits */
    } bit;

    uint32_t reg;
} sadc_r0_t;

/**
 * \brief           Sadc r1 register at offet 0x130
 */
typedef union sadc_r1_s {
    struct sadc_r1_b {
        uint32_t sadc_i_12b   : 12;             /*!< SADC output value after
                                                    digital control and No
                                                    oversampling */
        uint32_t reserved     : 4;
        uint32_t sadc_num_res : 16;             /*!< Number of SADC result
                                                    write into WDMA since last
                                                    sadc_start trigger */
    } bit;

    uint32_t reg;
} sadc_r1_t;

/**
 * \brief           Sadc r2 register at offet 0x134
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
    __IO sadc_ctrl0_t control0;                 /*!< 0x00 control register 0 */
    __IO sadc_ctrl1_t control1;                 /*!< 0x04 control register 1 */
    __IO sadc_ctrl2_t control2;                 /*!< 0x08 control register 2 */
    __IO sadc_set0_t setting0;                  /*!< 0x0C setting register 0 */
    __IO sadc_set1_t setting1;                  /*!< 0x10 setting register 1 */
    __IO uint32_t reserved1[3];                 /*!< 0x14 ~ 0x1C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch0;             /*!< 0x20 channel 0 pnsel register */
    __IO sadc_set_ch_t set_ch0;                 /*!< 0x24 channel 0 set register */
    __IO sadc_thd_ch_t thd_ch0;                 /*!< 0x28 channel 0 threshold register */
    __IO uint32_t reserved2;                    /*!< 0x2C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch1;             /*!< 0x30 channel 1 pnsel register */
    __IO sadc_set_ch_t set_ch1;                 /*!< 0x34 channel 1 set register */
    __IO sadc_thd_ch_t thd_ch1;                 /*!< 0x38 channel 1 threshold register */
    __IO uint32_t reserved3;                    /*!< 0x3C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch2;             /*!< 0x40 channel 2 pnsel register */
    __IO sadc_set_ch_t set_ch2;                 /*!< 0x44 channel 2 set register */
    __IO sadc_thd_ch_t thd_ch2;                 /*!< 0x48 channel 2 threshold register */
    __IO uint32_t reserved4;                    /*!< 0x4C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch3;             /*!< 0x50 channel 3 pnsel register */
    __IO sadc_set_ch_t set_ch3;                 /*!< 0x54 channel 3 set register */
    __IO sadc_thd_ch_t thd_ch3;                 /*!< 0x58 channel 3 threshold register */
    __IO uint32_t reserved5;                    /*!< 0x5C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch4;             /*!< 0x60 channel 4 pnsel register */
    __IO sadc_set_ch_t set_ch4;                 /*!< 0x64 channel 4 set register */
    __IO sadc_thd_ch_t thd_ch4;                 /*!< 0x68 channel 4 threshold register */
    __IO uint32_t reserved6;                    /*!< 0x6C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch5;             /*!< 0x70 channel 5 pnsel register */
    __IO sadc_set_ch_t set_ch5;                 /*!< 0x74 channel 5 set register */
    __IO sadc_thd_ch_t thd_ch5;                 /*!< 0x78 channel 5 threshold register */
    __IO uint32_t reserved7;                    /*!< 0x7C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch6;             /*!< 0x80 channel 6 pnsel register */
    __IO sadc_set_ch_t set_ch6;                 /*!< 0x84 channel 6 set register */
    __IO sadc_thd_ch_t thd_ch6;                 /*!< 0x88 channel 6 threshold register */
    __IO uint32_t reserved8;                    /*!< 0x8C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch7;             /*!< 0x90 channel 7 pnsel register */
    __IO sadc_set_ch_t set_ch7;                 /*!< 0x94 channel 7 set register */
    __IO sadc_thd_ch_t thd_ch7;                 /*!< 0x98 channel 7 threshold register */
    __IO uint32_t reserved9;                    /*!< 0x9C Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch8;             /*!< 0xA0 channel 8 pnsel register */
    __IO sadc_set_ch_t set_ch8;                 /*!< 0xA4 channel 8 set register */
    __IO sadc_thd_ch_t thd_ch8;                 /*!< 0xA8 channel 8 threshold register */
    __IO uint32_t reserved10;                   /*!< 0xAC Reserved */
    __IO sadc_pnsel_ch_t pnsel_ch9;             /*!< 0xB0 channel 9 pnsel register */
    __IO sadc_set_ch_t set_ch9;                 /*!< 0xB4 channel 9 set register */
    __IO sadc_thd_ch_t thd_ch9;                 /*!< 0xB8 channel 9 threshold register */
    __IO sadc_ana_set0_t ana_set0;              /*!< 0xBC analog setting register */
    __IO sadc_ana_set1_t ana_set1;              /*!< 0xC0 analog setting register */
    __IO uint32_t reserved11[15];               /*!< 0xC4 ~ 0xFC Reserved */
    __IO sadc_wdma_ctl0_t wdma_ctl0;            /*!< 0x100 wdma control register */
    __IO sadc_wdma_ctl1_t wdma_ctl1;            /*!< 0x104 wdma control register */
    __IO sadc_wdma_set0_t wdma_set0;            /*!< 0x108 wdma setting register 0 */
    __IO uint32_t wdma_set1;                    /*!< 0x10C wdma setting register 1 */
    __IO sadc_wdma_set2_t wdma_set2;            /*!< 0x110 wdma setting register 2 */
    __IO uint32_t wdma_r0;                      /*!< 0x114 wdma r0 register */
    __IO uint32_t wdma_r1;                      /*!< 0x118 wdma r1 register */
    __IO uint32_t reserved12;                   /*!< 0x11C Reserved */
    __IO sadc_int_t int_clear;                  /*!< 0x120 interrupt clear register */
    __IO sadc_int_t int_mask;                   /*!< 0x124 interrupt mask register */
    __IO sadc_int_t int_status;                 /*!< 0x128 interrupt status register */
    __IO sadc_r0_t r0;                          /*!< 0x12C r0 register */
    __IO sadc_r1_t r1;                          /*!< 0x130 r1 register */
    __IO sadc_r2_t r2;                          /*!< 0x134 r2 register */
} sadc_t;

#ifdef __cplusplus
}
#endif

#endif /* End of SADC_REG_H */

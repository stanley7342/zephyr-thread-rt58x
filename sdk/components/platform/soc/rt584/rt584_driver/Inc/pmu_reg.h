/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           pmu_reg.h
 * \brief          pmu register file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef PMU_REG_H
#define PMU_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           PMU bbpll read register at offet 0x08
 */
typedef union pmu_bbpll_read_s {
    struct pmu_bbpll_read_b {
        uint32_t bbpll_vtbit    : 2;            /*!< */
        uint32_t bbpll_tempbit  : 2;            /*!< */
        uint32_t bbpll_bank_vco : 3;            /*!< */
        uint32_t reserved       : 25;           /*!< */
    } bit;
    uint32_t reg;
} pmu_bbpll_read_t;

/**
 * \brief           PMU xtal 0 register at offet 0x20
 */
typedef union pmu_xtal0_s {
    struct pmu_xtal0_b {
        uint32_t en_xbuf           : 1;         /*!< */
        uint32_t en_d_xtalin       : 1;         /*!< */
        uint32_t sel_xbufload      : 1;         /*!< */
        uint32_t byp_xldo          : 1;         /*!< */
        uint32_t auto_pwd_xbuf     : 1;         /*!< */
        uint32_t xosc_en_inj       : 1;         /*!< */
        uint32_t xosc_en_vref1     : 1;         /*!< */
        uint32_t xosc_en_agc       : 1;         /*!< */
        uint32_t xosc_cap_maun     : 1;         /*!< */
        uint32_t xosc_cap_clk_div2 : 1;         /*!< */
        uint32_t xosc_bias_half    : 1;         /*!< */
        uint32_t xosc_iboost       : 1;         /*!< */
        uint32_t xosc_inj_time     : 2;         /*!< */
        uint32_t xosc_ampdet_th    : 2;         /*!< */
        uint32_t xosc_i_inj        : 2;         /*!< */
        uint32_t xosc_c_inj        : 2;         /*!< */
        uint32_t xosc_lpf_c        : 2;         /*!< */
        uint32_t xosc_lpf_r        : 1;         /*!< */
        uint32_t xosc_mode_xtal_bo : 1;         /*!< */
        uint32_t pw_xtal           : 3;         /*!< */
        uint32_t en_clockmodem     : 1;         /*!< */
        uint32_t reserved          : 4;         /*!< */
    } bit;
    uint32_t reg;
} pmu_xtal0_t;

/**
 * \brief           PMU xtal 1 register at offet 0x24
 */
typedef union pmu_xtal1_s {
    struct pmu_xtal1_b {
        uint32_t xosc_cap_ini           : 6;    /*!< */
        uint32_t reserved1              : 2;    /*!< */
        uint32_t xosc_cap_target        : 8;    /*!< */
        uint32_t cfg_xtal_settle_time   : 8;    /*!< */
        uint32_t cfg_bypass_xtal_settle : 1;    /*!< */
        uint32_t cfg_en_xtal_fast       : 1;    /*!< */
        uint32_t cfg_en_xtal            : 1;    /*!< */
        uint32_t cfg_en_xtal_auto       : 1;    /*!< */
        uint32_t reserved2              : 4;    /*!< */
    } bit;
    uint32_t reg;
} pmu_xtal1_t;

/**
 * \brief           PMU OSC 32k register at offet 0x34
 */
typedef union pmu_osc_32k_s {
    struct pmu_osc_32k_b {
        uint32_t tune_fine_rco_32k   : 8;       /*!< */
        uint32_t tune_coarse_rco_32k : 2;       /*!< */
        uint32_t pw_buf_rco_32k      : 2;       /*!< */
        uint32_t pw_rco_32k          : 4;       /*!< */
        uint32_t en_xo_32k           : 1;       /*!< */
        uint32_t en_xo_32k_fast      : 1;       /*!< */
        uint32_t pw_buf_xo_32k       : 2;       /*!< */
        uint32_t pw_xo_32k           : 3;       /*!< */
        uint32_t force_rco_32k_off   : 1;       /*!< */
        uint32_t rco_32k_sel         : 1;       /*!< */
        uint32_t reserved            : 7;       /*!< */
    } bit;
    uint32_t reg;
} pmu_osc_32k_t;

/**
 * \brief           PMU rvd0 register at offet 0x00
 */
typedef union pmu_rvd0_s {
    struct pmu_rvd0_b {
        uint32_t cfg_xtal_fast_time : 8;        /*!< */
        uint32_t cfg_ds_32k_off_dly : 5;        /*!< */
        uint32_t reserved           : 11;       /*!< */
        uint32_t pmu_reserved_0     : 8;        /*!< */
    } bit;
    uint32_t reg;
} pmu_rvd0_t;

/**
 * \brief           PMU rco 1M register at offet 0x8C
 */
typedef union pmu_rco1m_s {
    struct pmu_rco1m_b {
        uint32_t tune_fine_rco_1m   : 7;        /*!< */
        uint32_t reserved1          : 1;        /*!< */
        uint32_t tune_coarse_rco_1m : 4;        /*!< */
        uint32_t pw_rco_1m          : 2;        /*!< */
        uint32_t test_rco_1m        : 2;        /*!< */
        uint32_t en_rco_1m          : 1;        /*!< */
        uint32_t reserved2          : 15;       /*!< */
    } bit;
    uint32_t reg;
} pmu_rco1m_t;

/**
 * \brief           PMU dcdc vosel register at offet 0x90
 */
typedef union pmu_dcdc_vosel_s {
    struct pmu_dcdc_vosel_b {
        uint32_t dcdc_vosel_normal : 6;         /*!< */
        uint32_t reserved1         : 2;         /*!< */
        uint32_t dcdc_vosel_heavy  : 6;         /*!< */
        uint32_t reserved2         : 2;         /*!< */
        uint32_t dcdc_vosel_light  : 6;         /*!< */
        uint32_t reserved3         : 2;         /*!< */
        uint32_t dcdc_rup_rate     : 8;         /*!< */
    } bit;
    uint32_t reg;
} pmu_dcdc_vosel_t;

/**
 * \brief           PMU ldo mv vosel register at offet 0x94
 */
typedef union pmu_ldomv_vosel_s {
    struct pmu_ldomv_vosel_b {
        uint32_t ldomv_vosel_normal : 6;        /*!< */
        uint32_t reserved1          : 2;        /*!< */
        uint32_t ldomv_vosel_heavy  : 6;        /*!< */
        uint32_t reserved2          : 2;        /*!< */
        uint32_t ldomv_vosel_light  : 6;        /*!< */
        uint32_t reserved3          : 2;        /*!< */
        uint32_t dcdc_rdn_rate      : 8;        /*!< */
    } bit;
    uint32_t reg;
} pmu_ldomv_vosel_t;

/**
 * \brief           PMU core voasel register at offet 0x98
 */
typedef union pmu_core_vosel_s {
    struct pmu_core_vosel_b {
        uint32_t sldo_vosel_nm   : 6;           /*!< */
        uint32_t reserved1       : 2;           /*!< */
        uint32_t sldo_vosel_sp   : 6;           /*!< */
        uint32_t reserved2       : 2;           /*!< */
        uint32_t ldodig_vosel    : 4;           /*!< */
        uint32_t ldoflash_vosel  : 4;           /*!< */
        uint32_t ldoflash_sin    : 2;           /*!< */
        uint32_t ldoflash_ioc_wk : 1;           /*!< */
        uint32_t ldoflash_ioc_nm : 1;           /*!< */
        uint32_t por_dly         : 2;           /*!< */
        uint32_t por_vth         : 2;           /*!< */
    } bit;
    uint32_t reg;
} pmu_core_vosel_t;

/**
 * \brief           PMU dcdc normal register at offet 0x9C
 */
typedef union pmu_dcdc_normal_s {
    struct pmu_dcdc_normal_b {
        uint32_t dcdc_ppower_normal : 3;        /*!< */
        uint32_t dcdc_en_comp_normal: 1;        /*!< */
        uint32_t dcdc_npower_normal : 3;        /*!< */
        uint32_t dcdc_en_zcd_normal : 1;        /*!< */
        uint32_t dcdc_pdrive_normal : 3;        /*!< */
        uint32_t dcdc_mg_normal     : 1;        /*!< */
        uint32_t dcdc_ndrive_normal : 3;        /*!< */
        uint32_t dcdc_en_cm_normal  : 1;        /*!< */
        uint32_t dcdc_pw_normal     : 3;        /*!< */
        uint32_t dcdc_c_hg_normal   : 1;        /*!< */
        uint32_t dcdc_pwmf_normal   : 4;        /*!< */
        uint32_t dcdc_c_sc_normal   : 1;        /*!< */
        uint32_t dcdc_os_pn_normal  : 1;        /*!< */
        uint32_t dcdc_os_normal     : 2;        /*!< */
        uint32_t dcdc_hg_normal     : 2;        /*!< */
        uint32_t dcdc_dly_normal    : 2;        /*!< */
    } bit;
    uint32_t reg;
} pmu_dcdc_normal_t;

/**
 * \brief           PMU dcdc heavy register at offet 0xA0
 */
typedef union pmu_dcdc_heavy_s {
    struct pmu_dcdc_heavy_b {
        uint32_t dcdc_ppower_heavy  : 3;        /*!< */
        uint32_t dcdc_en_comp_heavy : 1;        /*!< */
        uint32_t dcdc_npower_heavy  : 3;        /*!< */
        uint32_t dcdc_en_zcd_heavy  : 1;        /*!< */
        uint32_t dcdc_pdrive_heavy  : 3;        /*!< */
        uint32_t dcdc_mg_heavy      : 1;        /*!< */
        uint32_t dcdc_ndrive_heavy  : 3;        /*!< */
        uint32_t dcdc_en_cm_heavy   : 1;        /*!< */
        uint32_t dcdc_pw_heavy      : 3;        /*!< */
        uint32_t dcdc_c_hg_heavy    : 1;        /*!< */
        uint32_t dcdc_pwmf_heavy    : 4;        /*!< */
        uint32_t dcdc_c_sc_heavy    : 1;        /*!< */
        uint32_t dcdc_os_pn_heavy   : 1;        /*!< */
        uint32_t dcdc_os_heavy      : 2;        /*!< */
        uint32_t dcdc_hg_heavy      : 2;        /*!< */
        uint32_t dcdc_dly_heavy     : 2;        /*!< */
    } bit;
    uint32_t reg;
} pmu_dcdc_heavy_t;

/**
 * \brief           PMU dcdc light register at offet 0xA4
 */
typedef union pmu_dcdc_light_s {
    struct pmu_dcdc_light_b {
        uint32_t dcdc_ppower_light  : 3;        /*!< */
        uint32_t dcdc_en_comp_light : 1;        /*!< */
        uint32_t dcdc_npower_light  : 3;        /*!< */
        uint32_t dcdc_en_zcd_light  : 1;        /*!< */
        uint32_t dcdc_pdrive_light  : 3;        /*!< */
        uint32_t dcdc_mg_light      : 1;        /*!< */
        uint32_t dcdc_ndrive_light  : 3;        /*!< */
        uint32_t dcdc_en_cm_light   : 1;        /*!< */
        uint32_t dcdc_pw_light      : 3;        /*!< */
        uint32_t dcdc_c_hg_light    : 1;        /*!< */
        uint32_t dcdc_pwmf_light    : 4;        /*!< */
        uint32_t dcdc_c_sc_light    : 1;        /*!< */
        uint32_t dcdc_os_pn_light   : 1;        /*!< */
        uint32_t dcdc_os_light      : 2;        /*!< */
        uint32_t dcdc_hg_light      : 2;        /*!< */
        uint32_t dcdc_dly_light     : 2;        /*!< */
    } bit;
    uint32_t reg;
} pmu_dcdc_light_t;

/**
 * \brief           PMU dcdc reserves register at offet 0xA8
 */
typedef union pmu_dcdc_reserved_s {
    struct pmu_dcdc_reserved_b {
        uint32_t dcdc_pw_dig_normal   : 2;      /*!< */
        uint32_t dcdc_pw_dig_heavy    : 2;      /*!< */
        uint32_t dcdc_pw_dig_light    : 2;      /*!< */
        uint32_t dcdc_ealv_auto       : 1;      /*!< */
        uint32_t dcdc_en_ealv         : 1;      /*!< */
        uint32_t dcdc_reserved_normal : 8;      /*!< */
        uint32_t dcdc_reserved_heavy  : 8;      /*!< */
        uint32_t dcdc_reserved_light  : 8;      /*!< */
    } bit;
    uint32_t reg;
} pmu_dcdc_reserved_t;

/**
 * \brief           PMU ldo control cause register at offet 0xAC
 */
typedef union pmu_ldo_ctrl_s {
    struct pmu_ldo_ctrl_b {
        uint32_t ldodig_sin              : 2;   /*!< */
        uint32_t ldodig_lout             : 1;   /*!< */
        uint32_t reserved1               : 1;   /*!< */
        uint32_t ldodig_ioc_nm           : 3;   /*!< */
        uint32_t reserved2               : 1;   /*!< */
        uint32_t ldomv_sin               : 2;   /*!< */
        uint32_t ldomv_lout              : 1;   /*!< */
        uint32_t ldoflash_vosel_init_sel : 1;   /*!< */
        uint32_t ldomv_ioc_nm            : 3;   /*!< */
        uint32_t reserved3               : 1;   /*!< */
        uint32_t dcdc_ioc                : 3;   /*!< */
        uint32_t dcdc_en_ocp             : 1;   /*!< */
        uint32_t dcdc_rup_en             : 1;   /*!< */
        uint32_t dcdc_rdn_en             : 1;   /*!< */
        uint32_t dcdc_manual_mode        : 2;   /*!< */
        uint32_t ldodig_ioc_wk           : 3;   /*!< */
        uint32_t reserved4               : 1;   /*!< */
        uint32_t ldomv_ioc_wk            : 3;   /*!< */
        uint32_t reserved5               : 1;   /*!< */
    } bit;
    uint32_t reg;
} pmu_ldo_ctrl_t;

/**
 * \brief           PMU enable control register at offet 0xB0
 */
typedef union pmu_en_ctrl_s {
    struct pmu_en_ctrl_b {
        uint32_t en_dcdc_nm      : 1;           /*!< */
        uint32_t en_ldomv_nm     : 1;           /*!< */
        uint32_t en_ldodig_nm    : 1;           /*!< */
        uint32_t en_bg_nm        : 1;           /*!< */
        uint32_t en_dcdc_sp      : 1;           /*!< */
        uint32_t en_ldomv_sp     : 1;           /*!< */
        uint32_t en_ldodig_sp    : 1;           /*!< */
        uint32_t en_bg_sp        : 1;           /*!< */
        uint32_t en_dcdc_ds      : 1;           /*!< */
        uint32_t en_ldomv_ds     : 1;           /*!< */
        uint32_t en_ldodig_ds    : 1;           /*!< */
        uint32_t en_bg_ds        : 1;           /*!< */
        uint32_t uvlo_dis_nm     : 1;           /*!< */
        uint32_t uvlo_dis_sp     : 1;           /*!< */
        uint32_t uvlo_dis_ds     : 1;           /*!< */
        uint32_t reserved1       : 1;           /*!< */
        uint32_t en_ldoana_nm    : 1;           /*!< */
        uint32_t en_ldoana_sp    : 1;           /*!< */
        uint32_t en_ldoana_ds    : 1;           /*!< */
        uint32_t reserved2       : 1;           /*!< */
        uint32_t en_ldoflash_nm  : 1;           /*!< */
        uint32_t en_ldoflash_sp  : 1;           /*!< */
        uint32_t en_ldoflash_ds  : 1;           /*!< */
        uint32_t reserved3       : 1;           /*!< */
        uint32_t ldoana_bypass   : 1;           /*!< */
        uint32_t reserved4       : 3;           /*!< */
        uint32_t en_ldoana_bg_nm : 1;           /*!< */
        uint32_t en_ldoana_bg_sp : 1;           /*!< */
        uint32_t en_ldoana_bg_ds : 1;           /*!< */
        uint32_t reserved5       : 1;           /*!< */
    } bit;
    uint32_t reg;
} pmu_en_ctrl_t;

/**
 * \brief           PMU bg control register at offet 0xB4
 */
typedef union pmu_bg_ctrl_s {
    struct pmu_bg_ctrl_b {
        uint32_t reserved1     : 16;            /*!< */
        uint32_t pmu_bg_os     : 2;             /*!< */
        uint32_t pmu_bg_os_dir : 1;             /*!< */
        uint32_t pmu_tc_bias   : 1;             /*!< */
        uint32_t pmu_mbulk     : 1;             /*!< */
        uint32_t pmu_bulk_manu : 1;             /*!< */
        uint32_t pmu_psrr      : 1;             /*!< */
        uint32_t reserved2     : 1;             /*!< */
        uint32_t pmu_ret_sel   : 1;             /*!< */
        uint32_t pmu_res_dis   : 1;             /*!< */
        uint32_t reserved3     : 6;             /*!< */
    } bit;
    uint32_t reg;
} pmu_bg_ctrl_t;

/**
 * \brief           PMU rf ldo register at offet 0xB8
 */
typedef union pum_rfldo_s {
    struct pum_rfldo_b {
        uint32_t ldoana_vtune_normal : 4;       /*!< */
        uint32_t ldoana_sin_m        : 2;       /*!< */
        uint32_t ldoana_bg_os        : 2;       /*!< */
        uint32_t ldoana_bg_os_dir    : 1;       /*!< */
        uint32_t ldoana_lout         : 1;       /*!< */
        uint32_t ldoana_bg_pn_sync   : 1;       /*!< */
        uint32_t ldoana_sel          : 1;       /*!< */
        uint32_t ldoana_ioc_nm       : 3;       /*!< */
        uint32_t reserved1           : 1;       /*!< */
        uint32_t ldoana_ioc_wk       : 3;       /*!< */
        uint32_t reserved2           : 1;       /*!< */
        uint32_t ldoana_vtune_heavy  : 4;       /*!< */
        uint32_t reserved3           : 6;       /*!< */
        uint32_t apmu_test           : 2;       /*!< */
    } bit;
    uint32_t reg;
} pmu_rfldo_t;

/**
 * \brief           PMU timing register at offet 0xBC
 */
typedef union pmu_timing_s {
    struct pmu_timing_b {
        uint32_t cfg_lv_settle_time      : 7;   /*!< */
        uint32_t cfg_bypass_lv_settle    : 1;   /*!< */
        uint32_t cfg_mv_settle_time      : 7;   /*!< */
        uint32_t cfg_bypass_mv_settle    : 1;   /*!< */
        uint32_t cfg_pwrx_settle_time    : 3;   /*!< */
        uint32_t reserved1               : 9;   /*!< */
        uint32_t force_dcdc_soc_pmu      : 1;   /*!< */
        uint32_t force_dcdc_soc_heavy_tx : 1;   /*!< */
        uint32_t force_dcdc_soc_light_rx : 1;   /*!< */
        uint32_t reserved2               : 1;   /*!< */
    } bit;
    uint32_t reg;
} pmu_timing_t;

/**
 * \brief           PMU bbpll 0 register at offet 0xC0
 */
typedef union pmu_bbpll0_s {
    struct pmu_bbpll0_b {
        uint32_t bbpll_setting_auto   : 1;      /*!< */
        uint32_t bbpll_selbbref_man   : 1;      /*!< */
        uint32_t bbpll_bank1_man      : 2;      /*!< */
        uint32_t bbpll_selbbclk_man   : 4;      /*!< */
        uint32_t bbpll_en_div_man     : 2;      /*!< */
        uint32_t bbpll_byp_ldo        : 1;      /*!< */
        uint32_t bbpll_manubank       : 1;      /*!< */
        uint32_t bbpll_trigger_bg     : 1;      /*!< */
        uint32_t bbpll_byp            : 1;      /*!< */
        uint32_t bbpll_sel_dly        : 1;      /*!< */
        uint32_t bbpll_sel_ib         : 1;      /*!< */
        uint32_t bbpll_sel_tc         : 1;      /*!< */
        uint32_t bbpll_en_vt_tempcomp : 1;      /*!< */
        uint32_t bbpll_sel_vth        : 2;      /*!< */
        uint32_t bbpll_sel_vtl        : 2;      /*!< */
        uint32_t bbpll_tune_temp      : 2;      /*!< */
        uint32_t bbpll_bg_os          : 2;      /*!< */
        uint32_t bbpll_hi             : 2;      /*!< */
        uint32_t bbpll_sel_icp        : 2;      /*!< */
        uint32_t bbpll_pw             : 2;      /*!< */
    } bit;
    uint32_t reg;
} pmu_bbpll0_t;

/**
 * \brief           PMU bbpll 1 register at offet 0xC4
 */
typedef union pmu_bbpll1_s {
    struct pmu_bbpll1_b {
        uint32_t bbpll_resetn_man      : 1;     /*!< */
        uint32_t bbpll_resetn_auto     : 1;     /*!< */
        uint32_t bbpll_bg_os_dir       : 1;     /*!< */
        uint32_t bbpll_out2_en         : 1;     /*!< */
        uint32_t bbpll_ini_bank        : 3;     /*!< */
        uint32_t reserved1             : 1;     /*!< */
        uint32_t cfg_bbpll_settle_time : 4;     /*!< */
        uint32_t bbpll_byp_ldo2        : 1;     /*!< */
        uint32_t bbpll_pn_sync         : 1;     /*!< */
        uint32_t pad_bbpll_out_en      : 1;     /*!< */
        uint32_t pad_xo32m_out_en      : 1;     /*!< */
        uint32_t reserved2             : 6;     /*!< */
    } bit;
    uint32_t reg;
} pmu_bbpll1_t;

/**
 * \brief           PMU reset cause register at offet 0xF0
 */
typedef union pmu_soc_ts_s {
    struct pmu_soc_ts_b     {
        uint32_t reserved0  : 16;               /*!< */
        uint32_t ts_vx      : 3;                /*!< */
        uint32_t reserved1  : 1;                /*!< */
        uint32_t ts_s       : 3;                /*!< */
        uint32_t reserved2  : 1;                /*!< */
        uint32_t ts_en      : 1;                /*!< */
        uint32_t ts_rst     : 1;                /*!< */
        uint32_t ts_clk_en  : 1;                /*!< */
        uint32_t reserved3  : 1;                /*!< */
        uint32_t ts_clk_sel : 2;                /*!< */
        uint32_t reserved4  : 2;                /*!< */
    } bit;
    uint32_t reg;
} pmu_soc_ts_t;


/**
 * \brief          PMU total register 
 */
typedef struct {
    __IO uint32_t            reserved0[2];          /*!< offset: 0x00~0x04 */
    __I  pmu_bbpll_read_t    soc_bbpll_read;        /*!< offset: 0x08 */
    __IO uint32_t            reserved0c;            /*!< offset: 0x0c */
    __IO uint32_t            reserved1[4];          /*!< offset: 0x10~0x1c */
    __IO pmu_xtal0_t         pmu_soc_pmu_xtal0;     /*!< offset: 0x20 */
    __IO pmu_xtal1_t         pmu_soc_pmu_xtal1;     /*!< offset: 0x24 */
    __IO uint32_t            reserved2[2];          /*!< offset: 0x28~0x2c */
    __IO uint32_t            reserved30;            /*!< offset: 0x30 */
    __IO pmu_osc_32k_t       pmu_osc32k;            /*!< offset: 0x34 */
    __IO uint32_t            reserved3[2];          /*!< offset: 0x38~0x3c */
    __IO pmu_rvd0_t          pmu_rvd0;              /*!< offset: 0x40 */
    __IO uint32_t            reserved4[3];          /*!< offset: 0x48~0x4c */
    __IO uint32_t            reserved5[4];          /*!< offset: 0x50~0x5c */
    __IO uint32_t            reserved6[4];          /*!< offset: 0x60~0x6c */
    __IO uint32_t            reserved7[4];          /*!< offset: 0x70~0x7c */
    __IO uint32_t            reserved8[3];          /*!< offset: 0x80~0x88 */
    __IO pmu_rco1m_t         soc_pmu_rco1m;         /*!< offset: 0x8c */
    __IO pmu_dcdc_vosel_t    pmu_dcdc_vosel;        /*!< offset: 0x90 */
    __IO pmu_ldomv_vosel_t   pmu_ldomv_vosel;       /*!< offset: 0x94 */
    __IO pmu_core_vosel_t    pmu_core_vosel;        /*!< offset: 0x98 */
    __IO pmu_dcdc_normal_t   pmu_dcdc_normal;       /*!< offset: 0x9c */
    __IO pmu_dcdc_heavy_t    pmu_dcdc_heavy;        /*!< offset: 0xa0 */
    __IO pmu_dcdc_light_t    pmu_dcdc_light;        /*!< offset: 0xa4 */
    __IO pmu_dcdc_reserved_t pmu_dcdc_reserved;     /*!< offset: 0xa8 */
    __IO pmu_ldo_ctrl_t      pmu_ldo_ctrl;          /*!< offset: 0xac */
    __IO pmu_en_ctrl_t       pmu_en_control;        /*!< offset: 0xb0 */
    __IO pmu_bg_ctrl_t       pmu_bg_control;        /*!< offset: 0xb4 */
    __IO pmu_rfldo_t         pmu_rfldo;             /*!< offset: 0xb8 */
    __IO pmu_timing_t        pmu_soc_pmu_timing;    /*!< offset: 0xbc */
    __IO pmu_bbpll0_t        soc_bbpll0;            /*!< offset: 0xc0 */
    __IO pmu_bbpll1_t        soc_bbpll1;            /*!< offset: 0xc4 */
    __IO uint32_t            reserved9;             /*!< offset: 0xc8 */
    __IO uint32_t            reserved10;            /*!< offset: 0xcc */
    __IO uint32_t            reserved11[4];         /*!< offset: 0xd0~0xdc */
    __IO uint32_t            reserved12[4];         /*!< offset: 0xe0~0xec */
    __IO pmu_soc_ts_t        soc_ts;                /*!< offset: 0xF0 */
} pmu_t;

#define PLL_LOCK_STATUS()                       (PMU_CTRL->soc_bbpll_read.bit.bbpll_vtbit)
#define PLL_VIBIT_STATUS()                      (PMU_CTRL->soc_bbpll_read.bit.bbpll_vtbit)
#define PLL_BANK_VCO_STATUS()                   (PMU_CTRL->soc_bbpll_read.bit.bbpll_bank_vco)

#ifdef __cplusplus
}
#endif

#endif /* End of PMU_REG_H */

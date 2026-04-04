/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            pmu_reg.h
 * \brief           pmu_reg.h include file
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

#include "chip_define.h"


/**
 * \brief           Pmu vsel voltage definitions
 */
enum {
    PMU_CONFIG_VSEL_0V95 = 0,                   /*!< Vsel = 0.95V. */
    PMU_CONFIG_VSEL_1V00 = 1,                   /*!< Vsel = 1.00V. */
    PMU_CONFIG_VSEL_1V05 = 2,                   /*!< Vsel = 1.05V. */
    PMU_CONFIG_VSEL_1V10 = 3,                   /*!< Vsel = 1.10V. */
    PMU_CONFIG_VSEL_1V15 = 4,                   /*!< Vsel = 1.15V. */
    PMU_CONFIG_VSEL_1V20 = 5,                   /*!< Vsel = 1.20V. */
    PMU_CONFIG_VSEL_1V25 = 6,                   /*!< Vsel = 1.25V. */
    PMU_CONFIG_VSEL_1V30 = 7,                   /*!< Vsel = 1.30V. */
};

/**
 * \brief           Pmu comparator 0 register at offet 0x00
 */
typedef union pmu_comp0_s {
    struct pmu_comp0_b {
        uint32_t aux_comp_selref       : 1;
        uint32_t aux_comp_int_en       : 1;
        uint32_t aux_comp_int_pol      : 2;
        uint32_t aux_comp_en_nm        : 1;
        uint32_t aux_comp_en_sp        : 1;
        uint32_t aux_comp_en_ds        : 1;
        uint32_t aux_comp_selinput     : 1;
        uint32_t aux_comp_vsel         : 4;
        uint32_t aux_comp_refsel       : 4;
        uint32_t aux_comp_chsel        : 4;
        uint32_t aux_comp_pw           : 2;
        uint32_t aux_comp_selhys       : 2;
        uint32_t aux_comp_swdiv        : 1;
        uint32_t aux_comp_psrr         : 1;
        uint32_t aux_comp_tc           : 1;
        uint32_t reserved              : 1;
        uint32_t aux_comp_ds_wakeup_en : 1;
        uint32_t aux_comp_ds_inv       : 1;
        uint32_t aux_en_start          : 2;
    } bit;

    uint32_t reg;
} pmu_comp0_t;

/**
 * \brief           Pmu comparator 1 register at offet 0x04
 */
typedef union pmu_comp1_s {
    struct pmu_comp1_b {
        uint32_t aux_comp_int_clr : 1;
        uint32_t reserved1        : 7;
        uint32_t bod_int_clr      : 1;
        uint32_t reserved2        : 23;
    } bit;

    uint32_t reg;
} pmu_comp1_t;

/**
 * \brief           Pmu comparator 2 register at offet 0x08
 */
typedef union pmu_comp2_s {
    struct pmu_comp2_b {
        uint32_t aux_comp_int_sta : 1;
        uint32_t aux_comp_out     : 1;
        uint32_t reserved1        : 6;
        uint32_t bod_int_sta      : 1;
        uint32_t bod_out          : 1;
        uint32_t reserved2        : 6;
        uint32_t vdd_det          : 3;
        uint32_t reserved3        : 13;
    } bit;

    uint32_t reg;
} pmu_comp2_t;

/**
 * \brief           Pmu TS register at offet 0x10
 */
typedef union pmu_ts_s {
    struct pmu_ts_b {
        uint32_t reserved0    : 16;
        uint32_t ts_vx        : 3;
        uint32_t reserved19   : 1;
        uint32_t ts_s         : 3;
        uint32_t reserved23   : 1;
        uint32_t ts_en        : 1;
        uint32_t ts_rst       : 1;
        uint32_t ts_clk_en    : 1;
        uint32_t reserved27   : 1;
        uint32_t ts_clk_sel   : 2;
        uint32_t test_aio_sel : 2;
    } bit;

    uint32_t reg;
} pmu_ts_t;

/**
 * \brief           Pmu pwr control register at offet 0x20
 */
typedef union pmu_pwr_ctrl_s {
    struct pmu_pwr_ctrl_b {
        uint32_t bod_int_en        : 1;
        uint32_t reserved1         : 1;
        uint32_t bod_int_pol       : 2;
        uint32_t reserved2         : 21;
        uint32_t cfg_en_vref1_xtal : 1;
        uint32_t cfg_sel_xbufload  : 1;
        uint32_t cfg_sel_xbufin    : 1;
        uint32_t cfg_ds_32k_off    : 1;
        uint32_t cfg_byp_xbuf_ldo  : 1;
        uint32_t en_xbuf           : 1;
        uint32_t en_d_xtalin       : 1;
    } bit;

    uint32_t reg;
} pmu_pwr_ctrl_t;

/**
 * \brief           Pmu xtal register at offet 0x24
 */
typedef union pmu_xtal_s {
    struct pmu_xtal_b {
        uint32_t cfg_xtal_settle_time   : 8;
        uint32_t cfg_pw_xtal            : 3;
        uint32_t cfg_bypass_xtal_settle : 1;
        uint32_t cfg_xtal_fast          : 1;
        uint32_t cfg_xtal_fast_auto     : 1;
        uint32_t cfg_en_xtal            : 1;
        uint32_t cfg_en_xtal_auto       : 1;
        uint32_t cfg_xtal_cap_sel       : 10;
        uint32_t reserved               : 6;
    } bit;

    uint32_t reg;
} pmu_xtal_t;

/**
 * \brief           Pmu mem control register at offet 0x28
 */
typedef union pmu_mem_ctrl_s {
    struct pmu_mem_ctrl_b {
        uint32_t reserved1      : 1;
        uint32_t cfg_sram_test1 : 1;
        uint32_t cfg_sram_rme   : 1;
        uint32_t reserved2      : 1;
        uint32_t cfg_sram_rm    : 4;
        uint32_t reserved3      : 10;
        uint32_t dly            : 2;
        uint32_t pwmf           : 3;
        uint32_t cm             : 1;
        uint32_t hg             : 2;
        uint32_t en_bod         : 1;
        uint32_t en_dect        : 1;
        uint32_t reserved4      : 1;
        uint32_t mg             : 1;
        uint32_t sin            : 2;
    } bit;

    uint32_t reg;
} pmu_mem_ctrl_t;

/**
 * \brief           Pmu rco32k register at offet 0x34
 */
typedef union pmu_rco32k_s {
    struct pmu_rco32k_b {
        uint32_t rn_32k     : 11;
        uint32_t reserved11 : 5;
        uint32_t pw32kxosc  : 3;
        uint32_t en_32kxosc : 1;
        uint32_t en_ldo32k  : 1;
        uint32_t reserved21 : 3;
        uint32_t c_32k      : 3;
        uint32_t tc_32k     : 1;
        uint32_t pw_32k     : 2;
        uint32_t psrr_32k   : 1;
        uint32_t sel_32k    : 1;
    } bit;

    uint32_t reg;
} pmu_rco32k_t;

/**
 * \brief           Pmu clock control register at offet 0x3C
 */
typedef union pmu_clk_ctrl_s {
    struct pmu_clk_ctrl_b {
        uint32_t reserved1        : 1;
        uint32_t en_clockmodem    : 1;
        uint32_t reserved2        : 10;
        uint32_t cfg_chip_en      : 1;
        uint32_t cfg_chip_en_auto : 1;
        uint32_t reserved3        : 18;
    } bit;

    uint32_t reg;
} pmu_clk_ctrl_t;

/**
 * \brief           Pmu pm select register at offet 0x48
 */
typedef union pmu_pm_sel_s {
    struct pmu_pm_sel_b {
        uint32_t reserved0            : 16;
        uint32_t xocap_update_mode    : 1;
        uint32_t reserved17           : 11;
        uint32_t cfg_pwrx_settle_time : 3;
        uint32_t reserved31           : 1;
    } bit;

    uint32_t reg;
} pmu_pm_sel_t;

/**
 * \brief           Pmu calbration 32k config 0 register at offet 0x50
 */
typedef union pmu_cal32k_cfg0_s {
    struct pmu_cal32k_cfg0_b {
        uint32_t cfg_cal32k_target : 18;
        uint32_t reserved18        : 6;
        uint32_t cfg_cal32k_en     : 1;
        uint32_t reserved25        : 7;
    } bit;

    uint32_t reg;
} pmu_cal32k_cfg0_t;

/**
 * \brief           Pmu calbration 32k config 1 register at offet 0x54
 */
typedef union pmu_cal32k_cfg1_s {
    struct pmu_cal32k_cfg1_b {
        uint32_t cfg_cal32k_lock_err    : 8;
        uint32_t cfg_cal32k_avg_coarse  : 2;
        uint32_t cfg_cal32k_avg_fine    : 2;
        uint32_t cfg_cal32k_avg_lock    : 2;
        uint32_t cfg_cal32k_dly         : 2;
        uint32_t cfg_cal32k_fine_gain   : 3;
        uint32_t cfg_cal32k_skip_coarse : 1;
        uint32_t cfg_cal32k_lock_gain   : 3;
        uint32_t cfg_cal32k_bound_mode  : 1;
        uint32_t cfg_cal32k_track_en    : 1;
        uint32_t en_ck_cal32k           : 1;
        uint32_t cfg_32k_est_en         : 1;
        uint32_t cfg_32k_rc_sel         : 1;
        uint32_t cfg_32k_est_mode       : 2;
        uint32_t cfg_32k_est_time       : 2;
    } bit;

    uint32_t reg;
} pmu_cal32k_cfg1_t;

/**
 * \brief           Pmu calbration 32k result 0 register at offet 0x58
 */
typedef union pmu_cal32k_r0_s {
    struct pmu_cal32k_r0_b {
        uint32_t est_32k_result       : 18;
        uint32_t reserved18           : 6;
        uint32_t est_32k_result_valid : 1;
        uint32_t reserved25           : 3;
        uint32_t cal32k_busy          : 1;
        uint32_t cal32k_lock          : 1;
        uint32_t cal32k_timeout       : 1;
        uint32_t reserved31           : 1;
    } bit;

    uint32_t reg;
} pmu_cal32k_r0_t;


/**
 * \brief           Pmu vout select 0 register at offet 0x90
 */
typedef union pmu_vout_sel0_s {
    struct pmu_vout_sel0_b {
        uint32_t dcdc_vosel_nm : 4;
        uint32_t dcdc_vosel_sp : 4;
        uint32_t dcdc_vosel_ds : 4;
        uint32_t reserved12    : 4;
        uint32_t lldo_vosel_nm : 4;
        uint32_t lldo_vosel_sp : 4;
        uint32_t lldo_vosel_ds : 4;
        uint32_t reserved28    : 4;
    } bit;

    uint32_t reg;
} pmu_vout_sel0_t;

/**
 * \brief           Pmu vout select 1 register at offet 0x94
 */
typedef union pmu_vout_sel1_s {
    struct pmu_vout_sel1_b {
        uint32_t sldo_vosel_nm : 4;
        uint32_t sldo_vosel_sp : 4;
        uint32_t sldo_vosel_ds : 4;
        uint32_t reserved12    : 20;
    } bit;

    uint32_t reg;
} pmu_vout_sel1_t;

/**
 * \brief           Pmu vout select 2 register at offet 0x98
 */
typedef union pmu_vout_sel2_s {
    struct pmu_vout_sel2_b {
        uint32_t ioldo_vosel_nm     : 4;
        uint32_t ioldo_vosel_sp     : 4;
        uint32_t ioldo_vosel_ds     : 4;
        uint32_t reserved12         : 4;
        uint32_t ioldo_ret_vosel_nm : 3;
        uint32_t reserved19         : 1;
        uint32_t ioldo_ret_vosel_sp : 3;
        uint32_t reserved23         : 1;
        uint32_t ioldo_ret_vosel_ds : 3;
        uint32_t reserved27         : 5;
    } bit;

    uint32_t reg;
} pmu_vout_sel2_t;

/**
 * \brief           Pmu bod select register at offet 0x9C
 */
typedef union pmu_bod_sel_s {
    struct pmu_bod_sel_b {
        uint32_t bod_r          : 2;
        uint32_t bod_f          : 2;
        uint32_t vdect          : 1;
        uint32_t reserved5      : 3;
        uint32_t apmu_test      : 2;
        uint32_t por_vth        : 2;
        uint32_t reserved12     : 4;
        uint32_t pmu_reserved_1 : 16;
    } bit;

    uint32_t reg;
} pmu_bod_sel_t;

/**
 * \brief           Pmu dcdc control 0 register at offet 0xA0
 */
typedef union pmu_dcdc_ctrl0_s {
    struct pmu_dcdc_ctrl0_b {
        uint32_t dcdc_ppower_heavy  : 3;
        uint32_t dcdc_en_comp_heavy : 1;
        uint32_t dcdc_npower_heavy  : 3;
        uint32_t dcdc_en_zcd_heavy  : 1;
        uint32_t dcdc_pdrive_heavy  : 3;
        uint32_t dcdc_mg_heavy      : 1;
        uint32_t dcdc_ndrive_heavy  : 3;
        uint32_t dcdc_cm_heavy      : 1;
        uint32_t dcdc_pw_heavy      : 2;
        uint32_t dcdc_c_sc_heavy    : 1;
        uint32_t dcdc_c_hg_heavy    : 1;
        uint32_t dcdc_pwmf_heavy    : 4;
        uint32_t reserved           : 1;
        uint32_t dcdc_os_pn_heavy   : 1;
        uint32_t dcdc_os_heavy      : 2;
        uint32_t dcdc_hg_heavy      : 2;
        uint32_t dcdc_dly_heavy     : 2;
    } bit;

    uint32_t reg;
} pmu_dcdc_ctrl0_t;

/**
 * \brief           Pmu dcdc control 1 register at offet 0xA4
 */
typedef union pmu_dcdc_ctrl1_s {
    struct pmu_dcdc_ctrl1_b {
        uint32_t dcdc_ppower_light  : 3;
        uint32_t dcdc_en_comp_light : 1;
        uint32_t dcdc_npower_light  : 3;
        uint32_t dcdc_en_zcd_light  : 1;
        uint32_t dcdc_pdrive_light  : 3;
        uint32_t dcdc_mg_light      : 1;
        uint32_t dcdc_ndrive_light  : 3;
        uint32_t dcdc_cm_light      : 1;
        uint32_t dcdc_pw_light      : 2;
        uint32_t dcdc_c_sc_light    : 1;
        uint32_t dcdc_c_hg_light    : 1;
        uint32_t dcdc_pwmf_light    : 4;
        uint32_t reserved           : 1;
        uint32_t dcdc_os_pn_light   : 1;
        uint32_t dcdc_os_light      : 2;
        uint32_t dcdc_hg_light      : 2;
        uint32_t dcdc_dly_light     : 2;
    } bit;

    uint32_t reg;
} pmu_dcdc_ctrl1_t;

/**
 * \brief           Pmu other control register at offet 0xAC
 */
typedef union pmu_other_ctrl_s {
    struct pmu_other_ctrl_b {
        uint32_t lldo_sin        : 2;
        uint32_t lldo_lout       : 1;
        uint32_t reserved3       : 1;
        uint32_t lldo_ioc        : 3;
        uint32_t lldo_en_ocp     : 1;
        uint32_t ioldo_sin       : 2;
        uint32_t ioldo_lout      : 1;
        uint32_t ioldo_cm        : 1;
        uint32_t ioldo_ret_sin   : 2;
        uint32_t ioldo_ret_lout  : 2;
        uint32_t dcdc_ioc        : 3;
        uint32_t dcdc_en_ocp     : 1;
        uint32_t dcdc_sw_heavy   : 1;
        uint32_t dcdc_auto_heavy : 1;
        uint32_t reserved22      : 2;
        uint32_t ioldo_ioc       : 3;
        uint32_t ioldo_en_ocp    : 1;
        uint32_t sel_bg0p6       : 1;
        uint32_t sel_bg0p95      : 1;
        uint32_t sel_bg1p2       : 1;
        uint32_t sel_iref        : 1;
    } bit;

    uint32_t reg;
} pmu_other_ctrl_t;

/**
 * \brief           Pmu enable control register at offet 0xB0
 */
typedef union pmu_en_ctrl_s {
    struct pmu_en_ctrl_b {
        uint32_t en_dcdc_nm    : 1;
        uint32_t en_lldo_nm    : 1;
        uint32_t en_ioldo_nm   : 1;
        uint32_t reserved3     : 1;
        uint32_t en_bg1_nm     : 1;
        uint32_t en_bg2_nm     : 1;
        uint32_t en_bod_nm     : 1;
        uint32_t en_dect_nm    : 1;
        uint32_t en_dcdc_sp    : 1;
        uint32_t en_lldo_sp    : 1;
        uint32_t en_ioldo_sp   : 1;
        uint32_t reserved11    : 1;
        uint32_t en_bg1_sp     : 1;
        uint32_t en_bg2_sp     : 1;
        uint32_t en_bod_sp     : 1;
        uint32_t en_dect_sp    : 1;
        uint32_t en_dcdc_ds    : 1;
        uint32_t en_lldo_ds    : 1;
        uint32_t en_ioldo_ds   : 1;
        uint32_t reserved19    : 1;
        uint32_t en_bg1_ds     : 1;
        uint32_t en_bg2_ds     : 1;
        uint32_t en_bod_ds     : 1;
        uint32_t en_dect_ds    : 1;
        uint32_t reserved24    : 1;
        uint32_t reserved25    : 1;
        uint32_t dis_iolod_ret : 1;
        uint32_t uvh_disable   : 1;
        uint32_t dis_por_nm    : 1;
        uint32_t dis_por_sp    : 1;
        uint32_t dis_por_ds    : 1;
        uint32_t reserved31    : 1;
    } bit;

    uint32_t reg;
} pmu_en_ctrl_t;

/**
 * \brief           Pmu bbpllr register at offet 0xBC
 */
typedef union pmu_bbpllr_s {
    struct pmu_bbpllr_b {
        uint32_t reserved0      : 24;
        uint32_t bbpll_bank_vco : 3;
        uint32_t reserved27     : 1;
        uint32_t bbpll_vtbit    : 2;
        uint32_t reserved30     : 2;
    } bit;

    uint32_t reg;
} pmu_bbpllr_t;

/**
 * \brief           Pmu bbpll register at offet 0xC4
 */
typedef union pmu_bbpll_s {
    struct pmu_bbpll_b {
        uint32_t reserved0         : 2;
        uint32_t bbpll_hi          : 2;
        uint32_t bbpll_ini_bank    : 2;
        uint32_t bbpll_pw          : 2;
        uint32_t bbpll_byp_ldo     : 1;
        uint32_t bbpll_manubank    : 1;
        uint32_t bbpll_trigger_bg  : 1;
        uint32_t bbpll_tp_vt       : 1;
        uint32_t bbpll_sel_dly     : 1;
        uint32_t bbpll_sel_icp     : 1;
        uint32_t bbpll_sel_tc      : 1;
        uint32_t bbpll_sel_vth     : 1;
        uint32_t bbpll_resetn_man  : 1;
        uint32_t bbpll_resetn_auto : 1;
        uint32_t bbpll_en_start    : 2;
        uint32_t bbpll_byp         : 1;
        uint32_t reserved21        : 1;
        uint32_t bbpll_bank1_man   : 1;
        uint32_t bbpll_bank1_auto  : 1;
        uint32_t en_uvl            : 1;
        uint32_t uvl_out_valid     : 1;
        uint32_t reserved26        : 2;
        uint32_t uvl_vth           : 2;
        uint32_t reserved30        : 2;
    } bit;

    uint32_t reg;
} pmu_bbpll_t;

/**
 * \brief           Pmu total register
 */
typedef struct {
    __IO pmu_comp0_t pmu_comp0;              //offset: 0x00
    __IO pmu_comp1_t pmu_comp1;              //offset: 0x04
    __IO pmu_comp2_t pmu_comp2;              //offset: 0x08
    __IO uint32_t pmu_reserved0c;            //offset: 0x0C
    __IO pmu_ts_t pmu_ts;                    //offset: 0x10
    __IO uint32_t pmu_rvd0;                  //offset: 0x14
    __IO uint32_t pmu_rvd1;                  //offset: 0x18
    __IO uint32_t pmu_reserved1c;            //offset: 0x1C
    __IO pmu_pwr_ctrl_t pmu_pwr_ctrl;        //offset: 0x20
    __IO pmu_xtal_t pmu_xtal;                //offset: 0x24
    __IO pmu_mem_ctrl_t pmu_mem_ctrl;        //offset: 0x28
    __IO uint32_t pmu_reserved2c;            //offset: 0x2C
    __IO uint32_t pmu_bod;                   //offset: 0x30
    __IO pmu_rco32k_t pmu_rco32k;            //offset: 0x34
    __IO uint32_t pmu_debug_ctrl;            //offset: 0x38
    __IO pmu_clk_ctrl_t pmu_clk_ctrl;        //offset: 0x3C
    __IO uint32_t pmu_gpio_drv;              //offset: 0x40
    __IO uint32_t pmu_reserved44;            //offset: 0x44
    __IO pmu_pm_sel_t pmu_pm_sel;            //offset: 0x48
    __IO uint32_t pmu_reserved4c;            //offset: 0x4C
    __IO pmu_cal32k_cfg0_t pmu_cal32k_cfg0;  //offset: 0x50
    __IO pmu_cal32k_cfg1_t pmu_cal32k_cfg1;  //offset: 0x54
    __IO pmu_cal32k_r0_t pmu_cal32k_result0; //offset: 0x58
    __IO uint32_t pmu_reserved5c;            //offset: 0x5C
    __IO uint32_t pmu_reserved60;            //offset: 0x60
    __IO uint32_t pmu_reserved64;            //offset: 0x64
    __IO uint32_t pmu_reserved68;            //offset: 0x68
    __IO uint32_t pmu_reserved6c;            //offset: 0x6C
    __IO uint32_t pmu_reserved70;            //offset: 0x70
    __IO uint32_t pmu_reserved74;            //offset: 0x74
    __IO uint32_t pmu_reserved78;            //offset: 0x78
    __IO uint32_t pmu_reserved7c;            //offset: 0x7C
    __IO uint32_t pmu_reserved80;            //offset: 0x80
    __IO uint32_t pmu_reserved84;            //offset: 0x84
    __IO uint32_t pmu_reserved88;            //offset: 0x88
    __IO uint32_t pmu_reserved8c;            //offset: 0x8C
    __IO pmu_vout_sel0_t pmu_vout_sel0;      //offset: 0x90
    __IO pmu_vout_sel1_t pmu_vout_sel1;      //offset: 0x94
    __IO pmu_vout_sel2_t pmu_vout_sel2;      //offset: 0x98
    __IO pmu_bod_sel_t pmu_bod_sel;          //offset: 0x9C
    __IO pmu_dcdc_ctrl0_t pmu_dcdc_ctrl0;    //offset: 0xA0
    __IO pmu_dcdc_ctrl1_t pmu_dcdc_ctrl1;    //offset: 0xA4
    __IO uint32_t pmu_reserveda8;            //offset: 0xA8
    __IO pmu_other_ctrl_t pmu_other_ctrl;    //offset: 0xAC
    __IO pmu_en_ctrl_t pmu_en_ctrl;          //offset: 0xB0
    __IO uint32_t pmu_reservedb4;            //offset: 0xB4
    __IO uint32_t pmu_reservedb8;            //offset: 0xB8
    __IO pmu_bbpllr_t pmu_bbpllr;            //offset: 0xBC
    __IO uint32_t pmu_reservedc0;            //offset: 0xC0
    __IO pmu_bbpll_t pmu_bbpll;              //offset: 0xC4
    __IO uint32_t pmu_reservedc8;            //offset: 0xC8
    __IO uint32_t pmu_reservedcc;            //offset: 0xCC
} pmu_t;

#define PLL_LOCK_STATUS()     (PMU->pmu_bbpllr.bit.bbpll_vtbit)
#define PLL_VIBIT_STATUS()    (PMU->pmu_bbpllr.bit.bbpll_vtbit)
#define PLL_BANK_VCO_STATUS() (PMU->pmu_bbpllr.bit.bbpll_bank_vco)

#ifdef __cplusplus
}
#endif

#endif /* End of PMU_REG_H */
